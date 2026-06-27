#!/usr/bin/env python3
"""docker-compose test client — receives 10 frames and validates JSON schema."""
import asyncio
import json
import os
import sys
import websockets

WS_URL = os.getenv("WS_URL", "ws://localhost:9001")

async def main() -> None:
    async with websockets.connect(WS_URL) as ws:
        for _ in range(10):
            raw  = await asyncio.wait_for(ws.recv(), timeout=10.0)
            data = json.loads(raw)
            assert "frame_id"    in data, f"Missing frame_id: {data}"
            assert "timestamp_ms" in data, f"Missing timestamp_ms: {data}"
            assert "contours"    in data and isinstance(data["contours"], list), \
                f"Bad contours: {data}"
            print(f"frame {data['frame_id']:>4d}: {len(data['contours'])} contours")
    print("OK — 10 frames received and validated")

asyncio.run(main())
