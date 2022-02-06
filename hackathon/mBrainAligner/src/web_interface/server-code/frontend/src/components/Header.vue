<template>
  <div class="header">
    <!-- <img
      src="../assets/images/header-logo.png"
      class="header-logo"
      
    /> -->
    <span class="header-text"><slot></slot></span>
    
  </div>
</template>

<script>
export default {
  
};
</script>

<style lang="less" scoped>
.header {
  height: 80px;
  line-height: 80px;
  width: 100%;
  background:white;
  top:0;
  // background: green;
  // margin-bottom: 100px;
  // top:2px;
  // left:0;
  // right:0;
  font-size: 26px;
  border-bottom: 1px solid #dddddd;
  text-align: center;
  position: fixed;
  z-index: 100;
  // position: sticky;
  // background: blue;
  .header-logo {
    height: 50px;
    opacity: 0.8;
    position: absolute;
    top: 50%;
    // top:30px;
    left: 10px;
    transform: translate(0, -50%);
    cursor: pointer;
  }
  .header-text {
    // position:absolute;
    font-weight: 400;
    font-size: 1.5em;
    vertical-align: middle;
    align-self: right;
    opacity: 0.8;
    // display: inline;
    // transform: translate(0, -50%);
    // color: green
  }
}
</style>
