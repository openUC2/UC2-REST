# dummy_caddy_like_server.py
from fastapi import FastAPI, Request, HTTPException
from fastapi.responses import (
    JSONResponse,
    HTMLResponse,
    PlainTextResponse,
    FileResponse,
)
from pathlib import Path
import stat

ROOT = Path("./").resolve()
ROOT.mkdir(exist_ok=True)

app = FastAPI()


def safe_path(req_path: str) -> Path:
    p = (ROOT / req_path).resolve()
    if not p.is_relative_to(ROOT):
        raise HTTPException(status_code=403)
    return p


def list_dir(path: Path):
    entries = []
    for p in sorted(path.iterdir()):
        st = p.stat()
        entries.append({
            "name": p.name,
            "type": "directory" if p.is_dir() else "file",
            "size": st.st_size if p.is_file() else 0,
            "mtime": int(st.st_mtime),
            "mode": stat.filemode(st.st_mode),
        })
    return entries


@app.get("/{req_path:path}")
async def browse(req_path: str, request: Request):
    fs_path = safe_path(req_path)

    if not fs_path.exists():
        raise HTTPException(status_code=404)

    # ---- FILE DOWNLOAD (binary-safe) ----
    if fs_path.is_file():
        return FileResponse(
            fs_path,
            filename=fs_path.name,
            media_type="application/octet-stream",
        )

    # ---- DIRECTORY ----
    # index.html suppresses listing
    index = fs_path / "index.html"
    if index.exists():
        return HTMLResponse(index.read_text())

    accept = request.headers.get("accept", "")
    entries = list_dir(fs_path)

    if "application/json" in accept:
        return JSONResponse(entries)

    if "text/plain" in accept:
        return PlainTextResponse("\n".join(e["name"] for e in entries))

    # default HTML
    html = "<ul>" + "".join(
        f"<li><a href='{e['name']}'>{e['name']}</a></li>"
        for e in entries
    ) + "</ul>"
    return HTMLResponse(html)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
