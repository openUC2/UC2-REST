import websocket
import json

url = "ws://192.168.4.1/ledarr_act"
payload = {
    "task": "/ledarr_act",
    "led": {
        "led_array": [{"id": 0, "r": 255, "g": 255, "b": 255}],
        "LEDArrMode": 1
    }
}

try:
    ws = websocket.WebSocket()
    ws.connect(url)

    # Convert the payload to JSON and send it
    ws.send(json.dumps(payload))
    print("Payload sent successfully")

    # You can also receive data from the WebSocket if needed
    response = ws.recv()
    print("Received response:", response)

    ws.close()
except Exception as e:
    print("WebSocket error:", str(e))
