using Microsoft.AspNetCore.SignalR;
using Microsoft.AspNetCore.Authorization;
using Core.Services;

namespace API.Hubs
{
    [Authorize]
    public class ChatHub : Hub
    {
        private readonly ChatService _chatService;

        public ChatHub(ChatService chatService)
        {
            _chatService = chatService;
        }

        public async Task SendMessage(string message)
        {
            var username = Context.User?.Identity?.Name ?? "Unknown";
            await _chatService.SaveMessageAsync(username, message);
            await Clients.All.SendAsync("ReceiveMessage", username, message);
        }
    }
}