using Data.Entities;

namespace Data.Repositories
{
    public interface IUserRepository
    {
        void AddUser(User user);
        User? GetByUsername(string username);
        void SaveChanges();
    }
}