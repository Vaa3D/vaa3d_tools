renderer1 = new THREE.WebGLRenderer({ alpha: false });

camera1= new THREE.PerspectiveCamera();
scene1 = new THREE.Scene();//不能和其他js的全局变量重名,同时函数也不能发生重名，否则会调用出错


function openlocal() {
  //大小先默认了一个值，后期确定整体图片大小再更改------------------------？？

    if (WEBGL.isWebGL2Available() === false) {

        document.body.appendChild(WEBGL.getWebGL2ErrorMessage());

    }
    document.getElementById('vaa3dweb-input-open-local').addEventListener('change', function (event) {
        //handleFileSelect();
        if (window.File && window.FileReader && window.FileList && window.Blob) {
            // Great success! All the File APIs are supported.
            var files = event.target.files;
            init1();
            for (var i = 0, f; f = files[i]; i++) {
                if (!f.type.match('tif.*')) {//只允许打开tif格式的文件
                    console.log('only open tif file!');
                    continue;
                }
                console.log(f.name);
                renderpicture(f);//目前考虑一张张图片渲染，计算图片位置？？？
                //render();
            }
           
        }
        else {
            alert('The File APIs are not fully supported in this browser.');
        }
    })
}

function helper() {
    bbox = new THREE.BoxGeometry(853, 537, 153);
    var edges = new THREE.EdgesGeometry(bbox);//添加边框
    var line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0xffffff }));
    line.position.set(427, 269, 77);
    scene1.add(line);

}
function init1() {
    // Create renderer
   
   

    width = document.getElementById('view').clientWidth;
    height = document.getElementById('view').clientHeight;
    renderer1.setSize(width, height);
    camera1 = new THREE.PerspectiveCamera(45, width / height, 1, 10000);
    controls = new THREE.OrbitControls(camera1, document.getElementById('view'));//创建控件对象
    controls.addEventListener('change', render);//监听鼠标、键盘事件
    controls.target.set(427, 269, 77);

    camera1.position.x = 431;
    camera1.position.y = 494;
    camera1.position.z = 1245;

  
    renderer1.setClearColor(0x000000, 1.0);
    renderer1.setPixelRatio(window.devicePixelRatio);//兼容高清屏幕
    document.getElementById("view").appendChild(renderer1.domElement);



    camera1.lookAt(427, 269, 77);

    var axesHelper = new THREE.AxesHelper(1000);
    scene1.add(axesHelper);
    helper();

   
    //console.log(cube);
    
    render1();

}
function render1() {
    requestAnimationFrame(render1);
    renderer1.render(scene1, camera1);

}

render1();

function renderpicture(file) {
    var filesname = file.name;
    var regex = /\d+/g;
    var filestr = filesname.match(regex);//将文件名显示的长宽高三个量分离到数组中去



    var reader = new FileReader();//读取本地文件
    reader.readAsArrayBuffer(file);
    reader.onload = (function (theFile) {
        return function (e) {
            var ifds = UTIF.decode(e.target.result);//解码tif文件，返回“IFD”数组（图像文件目录）,每一个ifd有xzy
            //console.log(ifds.length);
            UTIF.decodeImages(e.target.result, ifds);//循环通过每个IFD，并添加三个新的属性：width,height,data   
            //console.log(ifds);


            //将图片添加到场景中去
            var rgba = new Float32Array();
            rgba = UTIF.toRGBA8(ifds[0]);//以RGBA格式返回图像的Uint8Array，每个通道8位

            //var tiff = new Tiff({ buffer: file.data });//====================可能有问题
            //var count = tiff.countDirectory();

            var j;
            for (j = 1; j < ifds.length; j++) {//每一个PAGE就是一张二维图片 合起来就是三维图片了
                 var rgba= UTIF.toRGBA8(ifds[j]);
                 //-----------不知道需不需要这样

                var k; var contrast = 12; var thresh = 2;
                for (k = 0; k < rgba.byteLength; k = k + 4) {
                    if (rgba[k] < thresh) { rgba[k + 3] = 0; }
                    rgba[k] *= contrast;
                    rgba[k + 1] *= contrast;
                    rgba[k + 2] *= contrast;
                    if (rgba[k] == 0 && rgba[k + 1] == 0 && rgba[k + 1] == 0)
                        rgba[k + 3] = 0;

                }
                //console.log(filestr[1] / 114560);

               // console.log(rgba);

                // A begin
                var geometry = new THREE.Geometry(); //创建一个空几何体对象
                /**顶点坐标(纹理映射位置)*/

                console.log(filestr[0] + "" + filestr[1] + "" + filestr[2]);
                console.log(ifds[0].width + "..." + ifds[0].height);
                var p1 = new THREE.Vector3(0 + filestr[0] / 136960 * ifds[0].height, 0 + filestr[1] / 114560 * ifds[0].width, j + filestr[2] / 49280 * ifds.length); //顶点1坐标
                var p2 = new THREE.Vector3(0 + filestr[0] / 136960 * ifds[0].height, ifds[0].width + filestr[1] / 114560 * ifds[0].width, j + filestr[2] / 49280 * ifds.length); //顶点2坐标
                var p3 = new THREE.Vector3(ifds[0].height + filestr[0] / 136960 * ifds[0].height, ifds[0].width + filestr[1] / 114560 * ifds[0].width, j + filestr[2] / 49280 * ifds.length); //顶点3坐标
                var p4 = new THREE.Vector3(ifds[0].height + filestr[0] / 136960 * ifds[0].height, 0 + filestr[1] / 114560 * ifds[0].width, j + filestr[2] / 49280 * ifds.length); //顶点4坐标
                geometry.vertices.push(p1, p2, p3, p4); //顶点坐标添加到geometry对象
                //console.log(p1, p2, p3, p4);


                /** 三角面1、三角面2*/
                var normal = new THREE.Vector3(0, 0, 1); //三角面法向量
                var face0 = new THREE.Face3(0, 1, 2, normal); //三角面1
                var face1 = new THREE.Face3(0, 2, 3, normal); //三角面2
                geometry.faces.push(face0, face1); //三角面1、2添加到几何体
                /**纹理坐标*/
                var t0 = new THREE.Vector2(0, 0);//图片左下角
                var t1 = new THREE.Vector2(1, 0);//图片右下角
                var t2 = new THREE.Vector2(1, 1);//图片右上角
                var t3 = new THREE.Vector2(0, 1);//图片左上角
                uv1 = [t0, t1, t2];//选中图片一个三角区域像素——映射到三角面1
                uv2 = [t0, t2, t3];//选中图片一个三角区域像素——映射到三角面2
                geometry.faceVertexUvs[0].push(uv1, uv2);//纹理坐标传递给纹理三角面属性



                //var texture2 = new THREE.TextureLoader().load( rgba );

                //console.log(texture2);
                var texture = new THREE.DataTexture(rgba, ifds[0].width, ifds[0].height, THREE.RGBAFormat);//直接从原始数据和宽高来创建纹理。
                
                texture.needsUpdate = true;
               
                //console.log(texture);
                var material = new THREE.MeshBasicMaterial({ map: texture, side: THREE.DoubleSide });//有问题，不知道该用哪一种
                material.transparent = true;
                material.blending = THREE["CustomBlending"];//使用哪种混合
                material.blendEquation = THREE.MaxEquation;//混合使用使用时的方程
                material.alphaTest = 0.5;
                
                //material.depthWrite = false,
                //material.depthTest = false,
               // material.alphaTest = 0.5;

                var mesh = new THREE.Mesh(geometry, material);

                scene1.add(mesh);
               
            }
            
        }

       


    })(file);
    

    
    //window.addEventListener('resize', onWindowResize);
    function onWindowResize() {
        width = document.getElementById('view').clientWidth;
        height = document.getElementById('view').clientHeight;
        // 重新设置相机宽高比例
        camera1.aspect = width / height;
        // 更新相机投影矩阵
        camera1.updateProjectionMatrix();
        // 重新设置渲染器渲染范围
        renderer.setSize(width, height);
        setTimeout(renderer1.render(scene, camera), 300);
    }
   
   // document.getElementById('left-content').addEventListener('fullscreenchange', render);
}


