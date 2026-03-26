using Data.Entities;
using Microsoft.EntityFrameworkCore;
using Data.Context;

namespace Data.Repositories
{
    public class UserRepository : IUserRepository
    {
        private readonly AppDbContext _context;
        public UserRepository(AppDbContext context)
        {
            _context = context;
        }

        public void AddUser(User user)
        {
            _context.Users.Add(user);
        }

        public User? GetByUsername(string username)
        {
            return _context.Users
                .FirstOrDefault(u => u.Username == username);
        }

        public void SaveChanges()
        {
            _context.SaveChanges();
        }
    }
}