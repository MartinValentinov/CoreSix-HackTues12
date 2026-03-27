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
            if (dto.Alert is null)
                return BadRequest("Missing alert message");

            await _hub.Clients.All.SendAsync("ReceiveAlert", dto.Alert, dto.Distance);
            return Ok(new { received = dto.Distance, alert = dto.Alert });
        }
    }
}