using Data.Context;
using Data.Entities;
using MongoDB.Driver;
using MongoDB.Bson;

namespace Core.Services
{
    public class LikeUpdateResult
    {
        public string MessageId { get; set; } = string.Empty;
        public int Likes { get; set; }
        public bool LikedByCurrentUser { get; set; }
    }

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

        public async Task<List<ChatMessage>> GetMessagesForUserAsync(string username, int limit = 100)
        {
            var messages = await GetMessages(limit);
            var user = await _context.Users
                .Find(u => u.Username == username)
                .FirstOrDefaultAsync();

            var userKey = user?.Id;
            foreach (var message in messages)
            {
                message.LikedByCurrentUser =
                    !string.IsNullOrWhiteSpace(userKey) &&
                    message.LikedByUserIds.Contains(userKey);
            }

            return messages;
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

        public async Task<LikeUpdateResult> ToggleLikeAsync(string messageId, string username)
        {
            if (string.IsNullOrWhiteSpace(messageId))
            {
                throw new ArgumentException("Message id is required.", nameof(messageId));
            }

            if (!ObjectId.TryParse(messageId, out _))
            {
                throw new ArgumentException("Invalid message id format.", nameof(messageId));
            }

            var user = await _context.Users
                .Find(u => u.Username == username)
                .FirstOrDefaultAsync();

            if (string.IsNullOrWhiteSpace(user?.Id))
            {
                throw new InvalidOperationException("User not found.");
            }

            var message = await _context.Messages
                .Find(m => m.Id == messageId)
                .FirstOrDefaultAsync();

            if (message is null)
            {
                throw new InvalidOperationException("Message not found.");
            }

            var likedByCurrentUser = false;
            if (message.LikedByUserIds.Contains(user.Id))
            {
                message.LikedByUserIds.Remove(user.Id);
                message.Likes = Math.Max(0, message.Likes - 1);
            }
            else
            {
                message.LikedByUserIds.Add(user.Id);
                message.Likes += 1;
                likedByCurrentUser = true;
            }

            await _context.Messages.ReplaceOneAsync(m => m.Id == messageId, message);

            return new LikeUpdateResult
            {
                MessageId = messageId,
                Likes = message.Likes,
                LikedByCurrentUser = likedByCurrentUser
            };
        }

        public async Task SeedInitialMessagesAsync()
        {
            var anyMessages = await _context.Messages.Find(_ => true).AnyAsync();
            if (anyMessages)
            {
                return;
            }

            var now = DateTime.UtcNow;
            var seedMessages = new List<ChatMessage>
            {
                new()
                {
                    Username = "John D.",
                    Message = "Today I managed to walk 5 minutes without pain!",
                    Timestamp = now.AddHours(-2),
                    Likes = 3,
                    Comments = new List<ChatComment>
                    {
                        new()
                        {
                            Id = ObjectId.GenerateNewId().ToString(),
                            Username = "Anna K.",
                            Text = "Amazing progress!",
                            Timestamp = now.AddHours(-1)
                        }
                    }
                },
                new()
                {
                    Username = "Anna K.",
                    Message = "Adaptive exercises are changing my life.",
                    Timestamp = now.AddHours(-4),
                    Likes = 5,
                    Comments = new List<ChatComment>
                    {
                        new()
                        {
                            Id = ObjectId.GenerateNewId().ToString(),
                            Username = "Michael B.",
                            Text = "So inspiring!",
                            Timestamp = now.AddHours(-3)
                        }
                    }
                },
                new()
                {
                    Username = "Michael B.",
                    Message = "Small progress is still progress. Keep going everyone!",
                    Timestamp = now.AddHours(-6),
                    Likes = 4
                },
                new()
                {
                    Username = "Sarah L.",
                    Message = "Tried stretching before sleep - huge difference!",
                    Timestamp = now.AddDays(-1),
                    Likes = 6
                },
                new()
                {
                    Username = "David R.",
                    Message = "Grateful for this community",
                    Timestamp = now.AddDays(-1).AddHours(-1),
                    Likes = 7,
                    Comments = new List<ChatComment>
                    {
                        new()
                        {
                            Id = ObjectId.GenerateNewId().ToString(),
                            Username = "Anna K.",
                            Text = "Same here!",
                            Timestamp = now.AddHours(-20)
                        }
                    }
                },
                new()
                {
                    Username = "Emily T.",
                    Message = "Managed to stay consistent for a week!",
                    Timestamp = now.AddDays(-2),
                    Likes = 2
                }
            };

            await _context.Messages.InsertManyAsync(seedMessages);
        }
    }
}