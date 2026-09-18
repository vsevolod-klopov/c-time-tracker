import logging
import os
import threading
from typing import Any

from fastapi import Depends, FastAPI, Header, HTTPException, status
from pydantic import BaseModel

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
)
logger = logging.getLogger("data-server")

API_KEY = os.getenv("API_KEY", "secret")

app = FastAPI(title="Data Logger Server")

# Shared in-memory flag guarded by a lock, toggled via /flag endpoint.
_state_lock = threading.Lock()
_error_mode = False


def verify_api_key(x_api_key: str | None = Header(default=None, alias="X-API-Key")) -> None:
    if x_api_key != API_KEY:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid or missing API key",
        )


class FlagPayload(BaseModel):
    enabled: bool


@app.post("/data")
def receive_data(payload: dict[str, Any], _: None = Depends(verify_api_key)):
    with _state_lock:
        should_fail = _error_mode

    if should_fail:
        logger.error("Received data but error mode is enabled, rejecting: %s", payload)
        raise HTTPException(status_code=500, detail="Simulated error (error mode enabled)")

    logger.info("Received data: %s", payload)
    return {"status": "ok"}


@app.post("/flag")
def set_flag(payload: FlagPayload, _: None = Depends(verify_api_key)):
    global _error_mode
    with _state_lock:
        _error_mode = payload.enabled
        current = _error_mode

    logger.info("Error mode set to: %s", current)
    return {"error_mode": current}


@app.get("/flag")
def get_flag(_: None = Depends(verify_api_key)):
    with _state_lock:
        current = _error_mode
    return {"error_mode": current}


if __name__ == "__main__":
    import uvicorn

    uvicorn.run("main:app", host="0.0.0.0", port=8000, reload=True)
