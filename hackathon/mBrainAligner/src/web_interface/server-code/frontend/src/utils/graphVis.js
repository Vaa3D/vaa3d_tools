// g6引擎

// define data (load from json file )
var mydata = JSON.parse(jsondata)

// get element on the webpage
// const container = document.getElementById('myContainer');
// const width = container.scrollWidth;
// const height = container.scrollHeight || 600;
const width = 1000;
const height = 500;s
const Graph = new G6.Graph({
    container: 'myContainer',
    width,
    height,
    modes: {
        default: ['drag-canvas', 'zoom-canvas', 'drag-node'], //// 允许拖拽画布、放缩画布、拖拽节点
    },
    fitView: true, //设置是否将图适配到画布中；
    // 图布局
    layout: {
        type: 'grid', //格子布局，将节点有序（默认是数据顺序）排列在格子上。
        preventOverlap: true, // 防止节点重叠
    },
    //节点交互样式设置
    nodeStateStyles: {
        // 鼠标 hover 上节点，即 hover 状态为 true 时的样式
        hover: {
          fill: 'lightsteelblue',
        },
        // 鼠标点击节点，即 click 状态为 true 时的样式
        click: {
          stroke: '#000',
          lineWidth: 3,
        },
      },
      // 边不同状态下的样式集合
      edgeStateStyles: {
        // 鼠标点击边，即 click 状态为 true 时的样式
        click: {
          stroke: 'steelblue',
        },
      },
});

export {
    Graph
}


// const hull1 = graph.createHull({
//     id: 'hull1',
//     type: 'smooth-convex',
//     members: graph.getNodes().filter((node) => node.getModel().group === 1),
//     padding: 15,
//     style: {
//         fill: 'lightblue',
//     },
//     update: 'drag',
// });

// const hull2 = graph.createHull({
//     id: 'hull2',
//     type: 'smooth-convex',
//     members: graph.getNodes().filter((node) => node.getModel().group === 2),
//     padding: 15,
//     style: {
//         fill: 'pink',
//     },
//     update: 'drag',
// });

// graph.on('afterupdateitem', (e) => {
//     if (hull1.members.indexOf(e.item) > -1 || hull1.nonMembers.indexOf(e.item) > -1) {
//         hull1.updateData(hull1.members);
//     }
// });

// graph.on('afterupdateitem', (e) => {
//     if (hull2.members.indexOf(e.item) > -1 || hull2.nonMembers.indexOf(e.item) > -1) {
//         hull2.updateData(hull2.members);
//     }
// });

// if (typeof window !== 'undefined')
//     window.onresize = () => {
//         if (!graph || graph.get('destroyed')) return;
//         if (!container || !container.scrollWidth || !container.scrollHeight) return;
//         graph.changeSize(container.scrollWidth, container.scrollHeight);
//     };

// graph.data(mydata);
// graph.render();
