const signalR = require("@microsoft/signalr");

const token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9uYW1lIjoidGVzdCIsImV4cCI6MTc3NDU2MDA1MywiaXNzIjoiT09VLkFQSSIsImF1ZCI6Ik9PVS5DbGllbnRzIn0.LF732F2rwGZM3xYe1OB4uRScqC92V7y0cl3nVd-vEqg";

const connection = new signalR.HubConnectionBuilder()
    .withUrl("http://localhost:5239/chat", {
        accessTokenFactory: () => token
    })
    .build();


connection.on("ReceiveMessage", (user, message) => {
    console.log(user + ": " + message);
});

async function start() {
    try {
        await connection.start();
        console.log("SignalR Connected.");

        await connection.invoke("SendMessage", "Hello, Client me!");
    } catch (err) {
        console.error(err);    
    }
}

start();