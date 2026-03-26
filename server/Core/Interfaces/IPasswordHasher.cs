using Data.Entities;

namespace Core.Interfaces
{
    public interface IPasswordHasher
    {
        string HashPassword(User user, string password);
        bool VerifyPassword(User user, string password);
    }
}