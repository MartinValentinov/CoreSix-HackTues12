using MongoDB.Bson;
using MongoDB.Bson.Serialization.Attributes;

namespace Data.Entities
{
    public class ChatComment
    {
        [BsonRepresentation(BsonType.ObjectId)]
        public string? Id { get; set; }

        [BsonRepresentation(BsonType.ObjectId)]
        public string? UserId { get; set; }

        public string Username { get; set; } = "Unknown";
        public string Text { get; set; } = string.Empty;
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;
    }

    public class ChatMessage
    {
        [BsonId]
        [BsonRepresentation(BsonType.ObjectId)]
        public string? Id { get; set; }

        [BsonRepresentation(BsonType.ObjectId)]
        public string? UserId { get; set; }

        public string Username { get; set; } = "Unknown";
        public string Message { get; set; } = string.Empty;
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;
        public int Likes { get; set; } = 0;

        [BsonRepresentation(BsonType.ObjectId)]
        public List<string> LikedByUserIds { get; set; } = new();

        [BsonIgnore]
        public bool LikedByCurrentUser { get; set; } = false;

        public List<ChatComment> Comments { get; set; } = new();
    }
}