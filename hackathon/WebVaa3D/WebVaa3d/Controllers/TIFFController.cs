using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using System.IO;

namespace WebVaa3d.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class TIFFController : ControllerBase
    {
        const string TIFFResourceFolder = "TIFFSources";
        static bool HasTiff(string Name, out string tiffFolder)
        {
            var basePath = AppDomain.CurrentDomain.BaseDirectory;
            tiffFolder = Path.Combine(basePath, TIFFResourceFolder, Name);
            return Directory.Exists(tiffFolder);
        }
        static IEnumerable<string> GetTiffFilePath(string FolderPath)
            => Directory.GetFiles(FolderPath).Where(f => Path.GetExtension(f).ToLower().Contains("tif"));
        static IEnumerable<string> GetTiffFileNames(string FolderPath)
            => GetTiffFilePath(FolderPath).Select(f => Path.GetFileName(f)).OrderBy(n=> n);


        [Route("{name}")]
        public IActionResult Get(string Name)
        {
            if(HasTiff(Name, out string FolderPath))
            {
                return Ok(GetTiffFileNames(FolderPath));
            }
            return BadRequest("Invalid TIFF resouces");
        }

        [Route("{name}/{index}")]
        public IActionResult Get(string Name, string Index)
        {
            if (HasTiff(Name, out string FolderPath))
            {
                var files = GetTiffFilePath(FolderPath);
                var Target = Path.Combine(FolderPath, Index);
                if(files.Contains(Target))
                {
                    var fs = System.IO.File.OpenRead(Target);
                    return new FileStreamResult(fs, "image/tiff");
                }
                // Invalid Index
                return BadRequest("Invalid TIFF resouce");
            }
            // Invalid Name
            return BadRequest("Invalid TIFF resouce");
        }
    }
}