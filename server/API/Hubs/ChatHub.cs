using Microsoft.AspNetCore.SignalR;
using Microsoft.AspNetCore.Authorization;

namespace API.Hubs
{
    [Authorize]
    public class ChatHub : Hub
    {
        public async Task SendMessage(string message)
        {
            var username = Context.User?.Identity?.Name;

            await Clients.All.SendAsync("ReceiveMessage", username, message);
        }
    }
}