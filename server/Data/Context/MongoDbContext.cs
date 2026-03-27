using Data.Entities;
using MongoDB.Driver;
using Microsoft.Extensions.Configuration;
using System.IO;

namespace Data.Context
{
    public class MongoDbContext
    {
        private readonly IMongoDatabase _database;

        public MongoDbContext(IConfiguration config)
        {
            var connectionString =
                config["Mongo:ConnectionString"] ??
                config["Mongo__ConnectionString"] ??
                config["ConnectionStrings:Mongo"] ??
                config["MONGO_CONNECTION_STRING"];

            var databaseName =
                config["Mongo:DatabaseName"] ??
                config["Mongo__DatabaseName"] ??
                config["MONGO_DATABASE_NAME"];

            if (string.IsNullOrWhiteSpace(connectionString) || string.IsNullOrWhiteSpace(databaseName))
            {
                var envValues = LoadDotEnvValues();

                if (string.IsNullOrWhiteSpace(connectionString))
                {
                    connectionString =
                        envValues.GetValueOrDefault("Mongo:ConnectionString") ??
                        envValues.GetValueOrDefault("Mongo__ConnectionString") ??
                        envValues.GetValueOrDefault("MONGO_CONNECTION_STRING");
                }

                if (string.IsNullOrWhiteSpace(databaseName))
                {
                    databaseName =
                        envValues.GetValueOrDefault("Mongo:DatabaseName") ??
                        envValues.GetValueOrDefault("Mongo__DatabaseName") ??
                        envValues.GetValueOrDefault("MONGO_DATABASE_NAME");
                }
            }

            if (string.IsNullOrWhiteSpace(connectionString))
            {
                throw new Exception("MongoDB connection string is missing. Expected one of: Mongo:ConnectionString, Mongo__ConnectionString, ConnectionStrings:Mongo, MONGO_CONNECTION_STRING.");
            }

            if (string.IsNullOrWhiteSpace(databaseName))
            {
                throw new Exception("MongoDB database name is missing. Expected one of: Mongo:DatabaseName, Mongo__DatabaseName, MONGO_DATABASE_NAME.");
            }

            var client = new MongoClient(connectionString);
            _database = client.GetDatabase(databaseName);
        }

        private static Dictionary<string, string> LoadDotEnvValues()
        {
            var candidates = new[]
            {
                Path.Combine(Directory.GetCurrentDirectory(), ".env"),
                Path.Combine(Directory.GetCurrentDirectory(), "Data", ".env"),
                Path.Combine(Directory.GetCurrentDirectory(), "server", "Data", ".env"),
                Path.GetFullPath(Path.Combine(Directory.GetCurrentDirectory(), "..", "Data", ".env")),
                Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "Data", ".env"))
            };

            var envFile = candidates.FirstOrDefault(File.Exists);
            var values = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

            if (string.IsNullOrWhiteSpace(envFile))
            {
                return values;
            }

            foreach (var line in File.ReadLines(envFile))
            {
                var trimmed = line.Trim();
                if (string.IsNullOrWhiteSpace(trimmed) || trimmed.StartsWith("#"))
                {
                    continue;
                }

                var separatorIndex = trimmed.IndexOf('=');
                if (separatorIndex <= 0)
                {
                    continue;
                }

                var key = trimmed[..separatorIndex].Trim();
                var value = trimmed[(separatorIndex + 1)..].Trim();

                if (key.StartsWith("export ", StringComparison.OrdinalIgnoreCase))
                {
                    key = key[7..].Trim();
                }

                key = key.TrimStart('\uFEFF');

                if (!string.IsNullOrWhiteSpace(key))
                {
                    values[key] = value;
                }
            }

            return values;
        }

        public IMongoCollection<ChatMessage> Messages => 
            _database.GetCollection<ChatMessage>("Messages");

        public IMongoCollection<User> Users =>
            _database.GetCollection<User>("Users");

        public async Task EnsureIndexesAsync()
        {
            var messageIndexModels = new[]
            {
                new CreateIndexModel<ChatMessage>(
                    Builders<ChatMessage>.IndexKeys.Descending(m => m.Timestamp)),
                new CreateIndexModel<ChatMessage>(
                    Builders<ChatMessage>.IndexKeys.Ascending(m => m.UserId))
            };

            await Messages.Indexes.CreateManyAsync(messageIndexModels);

            var userIndex = new CreateIndexModel<User>(
                Builders<User>.IndexKeys.Ascending(u => u.Username),
                new CreateIndexOptions { Unique = true });

            await Users.Indexes.CreateOneAsync(userIndex);
        }
    }
}