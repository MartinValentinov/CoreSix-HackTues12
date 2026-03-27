using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.SignalR;
using API.Hubs;
using Core.DTOs;

namespace API.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class DistanceController : ControllerBase
    {
        private readonly IHubContext<AlertHub> _hub;

        public DistanceController(IHubContext<AlertHub> hub)
        {
            _hub = hub;
        }

        [HttpPost]
        public async Task<IActionResult> Post([FromBody] DistanceDto dto)
        {
            string message;

            if (dto.Distance < 20)
                message = "Obstacle very close!";
            else if (dto.Distance < 50)
                message = "Object ahead";
            else
                message = "Path clear";

            await _hub.Clients.All.SendAsync("ReceiveAlert", message);

            return Ok();
        }
    }
}