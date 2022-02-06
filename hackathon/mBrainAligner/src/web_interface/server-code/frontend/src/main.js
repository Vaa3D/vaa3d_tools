import Vue from "vue";
import App from "./App.vue";
import router from "./router";
import store from "./store";
import moment from 'moment'
// import "iview/dist/styles/iview.css";
// import "./assets/css/neo4jd3.min.css";
// import G6 from '@antv/g6';
import VueResource from 'vue-resource'
Vue.use(VueResource)


//适配移动端
import 'lib-flexible/flexible'
//国家插件
import CountryFlag from 'vue-country-flag'

// import echarts from 'echarts';

// import ViewUI from 'view-design';
import 'view-design/dist/styles/iview.css';
// Vue.use(ViewUI);
import { Input, Button, Icon, Message, Select, Option, Divider, Table, Modal, Form, FormItem, RadioGroup, Radio, ListItem, List, Page, Upload, Tooltip, Row, Col, Notice, InputNumber, Checkbox, CheckboxGroup, Progress } from "view-design";
Vue.component("Input", Input);
Vue.component("Button", Button);
Vue.component("Icon", Icon);
Vue.component("Message", Message);
Vue.component("Select", Select);
Vue.component("Option", Option);
Vue.component("Divider", Divider);
Vue.component("Table", Table);
Vue.component("Modal", Modal);
Vue.component("Form", Form);
Vue.component("FormItem", FormItem);
Vue.component("RadioGroup", RadioGroup);
Vue.component("Radio", Radio);
Vue.component('List', List);
Vue.component('ListItem', List.Item);
Vue.component("Page", Page);
Vue.component("Upload", Upload);
Vue.component("Tooltip", Tooltip);
Vue.component("Row", Row);
Vue.component("Col", Col);
Vue.component("Notice", Notice);
Vue.component("InputNumber", InputNumber);
Vue.component("Checkbox", Checkbox);
Vue.component("CheckboxGroup", CheckboxGroup);
Vue.component("CountryFlag", CountryFlag);
Vue.component("Progress", Progress);


Vue.prototype.$Message = Message;

import axios from "axios"
Vue.prototype.$axios = axios;

Vue.prototype.$Message.config({
  top: 100,
  // left: 100,
  duration: 1
});
Vue.prototype.$Modal = Modal;

// Vue.prototype.$echarts = echarts;
Vue.prototype.$moment = moment;

// 设置请求超时
Vue.http.interceptors.push((request, next) => {
  let timeout;
  // 這裡改用 _timeout
  if (request._timeout) {
    timeout = setTimeout(() => {　　　　　　　　//自定义响应体 status:408,statustext:"请求超时"，并返回给下下边的next
      next(request.respondWith(request.body, {
        status: 408,
        statusText: '请求超时'
      }));

    }, request._timeout);
  }
  next((response) => {
    console.log(response.status)//如果超时输出408　　　　return response;
  })
})
// 设置请求超时


Vue.config.productionTip = false;
// Vue.prototype.Neo4jd3 = Neo4jd3;
// Vue.prototype.d3 = d3;
new Vue({
  router,
  store,
  render: h => h(App)
}).$mount("#app");
