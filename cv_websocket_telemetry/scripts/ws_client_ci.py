#!/usr/bin/env python3
"""CI smoke test — server must be running on localhost:9001."""
import asyncio
import json
import sys
import websockets

async def smoke() -> None:
    try:
        async with websockets.connect("ws://localhost:9001", open_timeout=8) as ws:
            raw  = await asyncio.wait_for(ws.recv(), timeout=15.0)
            data = json.loads(raw)
            assert "frame_id" in data
            assert "contours" in data
            print(f"OK: frame_id={data['frame_id']} contours={len(data['contours'])}")
    except Exception as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        sys.exit(1)

asyncio.run(smoke())
