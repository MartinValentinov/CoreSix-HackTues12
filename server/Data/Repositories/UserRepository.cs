using Data.Entities;
using Data.Context;
using MongoDB.Driver;

namespace Data.Repositories
{
    public class UserRepository : IUserRepository
    {
        private readonly MongoDbContext _context;
        public UserRepository(MongoDbContext context)
        {
            _context = context;
        }

        public void AddUser(User user)
        {
            _context.Users.InsertOne(user);
        }

        public User? GetByUsername(string username)
        {
            return _context.Users
                .Find(u => u.Username == username)
                .FirstOrDefault();
        }

        public void SaveChanges()
        {
            // MongoDB writes are immediate; kept for interface compatibility.
        }
    }
}