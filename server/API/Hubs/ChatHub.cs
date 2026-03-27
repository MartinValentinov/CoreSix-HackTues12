using Microsoft.AspNetCore.SignalR;
using Microsoft.AspNetCore.Authorization;
using Core.Services;
using Data.Entities;

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
            var saved = await _chatService.SaveMessageAsync(username, message);
            await Clients.All.SendAsync("ReceiveMessage", saved);
        }

        public async Task<List<ChatMessage>> GetRecentMessages()
        {
            var username = Context.User?.Identity?.Name ?? "Unknown";
            return await _chatService.GetMessagesForUserAsync(username);
        }

        public async Task AddComment(string messageId, string commentText)
        {
            var username = Context.User?.Identity?.Name ?? "Unknown";
            var comment = await _chatService.AddCommentAsync(messageId, username, commentText);

            await Clients.All.SendAsync("ReceiveComment", new
            {
                MessageId = messageId,
                Comment = comment
            });
        }

        public async Task ToggleLike(string messageId)
        {
            var username = Context.User?.Identity?.Name ?? "Unknown";
            var result = await _chatService.ToggleLikeAsync(messageId, username);

            await Clients.All.SendAsync("ReceiveLikeUpdate", new
            {
                MessageId = result.MessageId,
                Likes = result.Likes,
                LikedByCurrentUser = result.LikedByCurrentUser
            });
        }
    }
}