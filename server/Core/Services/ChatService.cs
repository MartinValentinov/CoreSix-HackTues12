using Data.Context;
using Data.Entities;
using MongoDB.Driver;
using MongoDB.Bson;

namespace Core.Services
{
    public class ChatService
    { 
        private readonly MongoDbContext _context;

        public ChatService(MongoDbContext context)
        {
            _context = context;
        }

        public async Task<ChatMessage> SaveMessageAsync(string username, string message)
        {
            var normalizedMessage = message?.Trim() ?? string.Empty;
            if (string.IsNullOrWhiteSpace(normalizedMessage))
            {
                throw new ArgumentException("Message cannot be empty.", nameof(message));
            }

            var user = await _context.Users
                .Find(u => u.Username == username)
                .FirstOrDefaultAsync();

            var msg = new ChatMessage
            {
                UserId = user?.Id,
                Username = username,
                Message = normalizedMessage,
                Timestamp = DateTime.UtcNow
            };

            await _context.Messages.InsertOneAsync(msg);
            return msg;
        }

        public async Task<List<ChatMessage>> GetMessages(int limit = 100)
        {
            return await _context.Messages
                .Find(_ => true)
                .SortByDescending(m => m.Timestamp)
                .Limit(Math.Max(1, limit))
                .ToListAsync();
        }

        public async Task<ChatComment> AddCommentAsync(string messageId, string username, string text)
        {
            if (string.IsNullOrWhiteSpace(messageId))
            {
                throw new ArgumentException("Message id is required.", nameof(messageId));
            }

            var normalizedText = text?.Trim() ?? string.Empty;
            if (string.IsNullOrWhiteSpace(normalizedText))
            {
                throw new ArgumentException("Comment cannot be empty.", nameof(text));
            }

            if (!ObjectId.TryParse(messageId, out _))
            {
                throw new ArgumentException("Invalid message id format.", nameof(messageId));
            }

            var user = await _context.Users
                .Find(u => u.Username == username)
                .FirstOrDefaultAsync();

            var comment = new ChatComment
            {
                Id = ObjectId.GenerateNewId().ToString(),
                UserId = user?.Id,
                Username = username,
                Text = normalizedText,
                Timestamp = DateTime.UtcNow
            };

            var updateResult = await _context.Messages.UpdateOneAsync(
                m => m.Id == messageId,
                Builders<ChatMessage>.Update.Push(m => m.Comments, comment));

            if (updateResult.MatchedCount == 0)
            {
                throw new InvalidOperationException("Message not found.");
            }

            return comment;
        }
    }
}