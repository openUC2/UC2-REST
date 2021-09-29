from fastapi import FastAPI, Request, Body
import uvicorn

app = FastAPI()


@app.get("/")
async def read_root():
    return {"Hello": "World"}

@app.get("/read_lens")
def home():
    return {"Hello": "FastAPI"}

@app.post('/move_y')
async def create_contact(contact):
    print(contact)
    return contact

@app.post("/move_z")
async def get_body(request: Request):
    print(request.json().__dict__)
    return await request.json()

@app.post('/move_x')
async def update_item(payload: dict = Body(...)):
    print(payload)    
    return payload

@app.post('/lens_x')
async def update_item2(payload: dict = Body(...)):
    print(payload)    
    return payload

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=80, log_level="info")