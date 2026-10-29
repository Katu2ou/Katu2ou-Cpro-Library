#!/usr/bin/env python3
"""検証用の提出コードを Library Checker に匿名提出し、結果を保存する。

python3 submit.py [--only key ...]
POST 前後に状態を保存。同一ソースの再実行は判定取得を再開するだけ。
POST の成否が不明な記録は自動再提出せず、手動確認を要求する。
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import time
import urllib.error
import urllib.request

HERE = Path(__file__).resolve().parent
LIB = HERE.parent.parent
API = 'https://v3.api.judge.yosupo.jp'
FINAL = {'AC','WA','RE','TLE','MLE','CE','OLE','IE'}
RESULT = HERE/'online-results.json'


def now():
    return datetime.now(timezone.utc).isoformat()


def request(path, payload=None):
    data = None if payload is None else json.dumps(payload).encode()
    req = urllib.request.Request(API+path, data=data, headers={
        'Content-Type':'application/json', 'User-Agent':'katu2ou-fps-verification/1.0'})
    with urllib.request.urlopen(req, timeout=40) as response:
        return json.load(response)


def save(records):
    temp = RESULT.with_suffix('.tmp')
    temp.write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
    temp.replace(RESULT)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--only',nargs='+')
    args=parser.parse_args()
    tasks=json.loads((HERE/'manifest.json').read_text())
    if args.only:
        tasks=[t for t in tasks if t['key'] in args.only]
    records=json.loads(RESULT.read_text()) if RESULT.exists() else []
    queue=[]
    pending=[]
    for task in tasks:
        raw=(HERE/task['source']).read_bytes()
        assert hashlib.sha256(raw).hexdigest()==task['source_sha256']
        for name, sha in task['libraries'].items():
            assert hashlib.sha256((LIB/name).read_bytes()).hexdigest()==sha, name
        previous=[r for r in records if r['key']==task['key'] and r['source_sha256']==task['source_sha256']]
        if previous:
            record=previous[-1]
            if 'id' not in record:
                raise RuntimeError('Uncertain earlier POST; inspect before retry: '+task['key'])
            if record['status'] not in FINAL:
                pending.append(record)
        else:
            queue.append(task)
    # 修正前のソースに未取得の判定がある場合も、再提出せず最後まで記録する。
    active_ids={r['id'] for r in pending}
    for record in records:
        if 'id' in record and record['status'] not in FINAL and record['id'] not in active_ids:
            pending.append(record)
            active_ids.add(record['id'])
    (HERE/'judge').mkdir(exist_ok=True)
    while queue or pending:
        while queue and len(pending)<2:
            task=queue.pop(0)
            record=dict(task, started_at=now(),status='SUBMITTING',anonymous=True)
            records.append(record)
            save(records)
            try:
                answer=request('/submit',dict(problem=task['problem'],lang=task['lang'],
                    source=(HERE/task['source']).read_text(),tle_knockout=True))
                record['id']=answer['id']
            except Exception as exc:
                record['status']='SUBMISSION_UNKNOWN'
                record['error']=str(exc)
                save(records)
                raise
            record.update(status='WJ',url=f"https://judge.yosupo.jp/submission/{record['id']}")
            save(records)
            pending.append(record)
            print('SUBMITTED',task['key'],record['url'],flush=True)
            time.sleep(1)
        for record in pending[:]:
            try:
                result=request(f"/submissions/{record['id']}")
            except Exception as exc:
                print('POLL_RETRY',record['id'],str(exc),flush=True)
                continue
            assert hashlib.sha256(result['source'].encode()).hexdigest()==record['source_sha256']
            record.update(source_verified=True,overview=result['overview'],
                          status=result['overview']['status'],checked_at=now())
            (HERE/'judge'/f"{record['id']}.json").write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
            save(records)
            if record['status'] in FINAL:
                print('RESULT',record['key'],record['status'],record['url'],flush=True)
                pending.remove(record)
        if pending:
            time.sleep(5)
    print('DONE',len(tasks),'adapters.',flush=True)


if __name__=='__main__':
    main()
