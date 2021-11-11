import axios from "@/axios/index";

// 根路径
const initPage = () =>
  axios(
    {
      url: "/",
      method: "GET",
    }
  )


const downloadSample = (sample_opt_current) => axios({
  url: "/sample_data",
  method: "GET",
  params: {
    sample_opt_current,
  }
})

const downloadRes = (data) => axios(
  // 下载result
  {
    url: "/download_result",
    method: "POST",
    responseType: 'blob',
    data: data,
  }
)

const upload = () => axios(
  // 上传图片
  {
    url: "upload",
    method: "POST"
  }
)
const downloadStatus = () => axios({
  url: "download_status",
  method: "GET"
})

const register = (data) => axios({
  url: "/register",
  method: "POST",
  data: data
})


const runCmd = (cmd_data) => axios({
  url: "/run_cmd",
  method: "POST",
  data: cmd_data,
  headers: {
    'Content-Type': 'application/x-www-form-urlencoded',
    // 'Content-Type': 'application/json;charset=UTF-8',
    'Accept': 'application/json'
  }
})



// write_config
const write_config = (config_text) =>
  axios(
    {
      url: "/write_config",
      method: "GET",
      params: {
        config_text
      }
    }
  )


const log = (path, folder_name) => axios({
  url: "/log",
  method: "GET",
  params:{
    path,
    folder_name
  }
})

const api = {
  initPage,
  upload,
  write_config,
  downloadRes,
  runCmd, // run cmd
  log, //日志
  downloadStatus, //是否可以下载了
  register,
  downloadSample

};
export default api;
