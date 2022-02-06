import axios from "axios";
import "../../node_modules/nprogress/nprogress.css";
import NProgress from "nprogress";
import Vue from "vue";
const instance = axios.create({
  baseURL: "/api",
  timeout: 60 * 60 * 1000 , //10小时
  headers: {
    // "Content-Type": "application/json;charset=UTF-8",
    // "Access-Control-Allow-Origin": "*",
    'Content-Type': 'application/x-www-form-urlencoded',
    'Accept': 'application/json'
  }
});
instance.interceptors.request.use(
  config => {
    // if (config.method === 'post') {
    //     //如果是post请求则进行序列化处理
    //     config.data = qs.stringify(config.data);
    // }
    // const token = localStorage.getItem("token");
    // config.headers.common["Authorization"] = "Bearer " + token;
    NProgress.start();

    return config;
  },
  error => {
    return Promise.reject(error);
  }
);
// instance.interceptors.request.use(
//   config => {
//     if (config.method === 'post') {
//     //     //如果是post请求则进行序列化处理
//         // config.data = qs.stringify(config.data);
//     }
//     // const token = localStorage.getItem("token");
//     // config.headers.common["Authorization"] = "Bearer " + token;
//     NProgress.start();
//     console.log("config: ", config);

//     return config;
//   },
//   error => {
//     return Promise.reject(error);
//   }
// );

// instance.interceptors.response.use(
//   res => {
//     NProgress.done();
//     if (res.data.code === 1) {
//       Vue.prototype.$Message.warning(res.data.message);
//     }
//     return res;
//   },
//   error => {
//     //404等问题可以在这里处理
//     NProgress.done();
//     // if (error.response.status === 401) {
//     //   localStorage.clear();
//     //   // message.error('登录信息过期，请重新登录');
//     // } else if (error.response.status === 500) {
//     //   // message.error('500 报错，请检查网络')
//     // }

//     // if (error.response.status === 401) {
//     //   localStorage.clear();
//     // } else if (error.response.status === 500) {
//     // }

//     return Promise.reject(error);
//   }
// );
export default instance;
