using Data.Context;
using Data.Entities;
using MongoDB.Driver;

namespace Core.Services
{
    public class ChatService
    { 
        private readonly MongoDbContext _context;

        public ChatService(MongoDbContext context)
        {
            _context = context;
        }

        public async Task SaveMessageAsync(string username, string message)
        {
            var msg = new ChatMessage
            {
                Username = username,
                Message = message,
                Timestamp = DateTime.UtcNow
            };

            await _context.Messages.InsertOneAsync(msg);
        }

        public async Task<List<ChatMessage>> GetMessages()
        {
            return await _context.Messages
                .Find(_=> true)
                .SortByDescending(m => m.Timestamp)
                .Limit(50)
                .ToListAsync();
        }
    }
}