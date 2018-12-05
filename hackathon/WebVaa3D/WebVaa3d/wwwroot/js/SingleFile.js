function show(file) {
    renderer = new THREE.WebGLRenderer({ alpha: false });
    width = document.getElementById('view').clientWidth;
    height = document.getElementById('view').clientHeight;
    renderer.setSize(width, height);
    camera = new THREE.PerspectiveCamera(45, width / height, 1, 10000);
    scene= new THREE.Scene;
    controls = new THREE.OrbitControls(camera, document.getElementById('view'));//创建控件对象
    controls.addEventListener('change', render);//监听鼠标、键盘事件

    var axesHelper = new THREE.AxesHelper(1000);
    scene.add(axesHelper);

    function render() {
        //requestAnimationFrame(render);
        renderer.render(scene, camera);

    }



    //console.log(cube);
    render();

    //renderer.setClearColor(0x000000, 1.0);
    renderer.setClearColor(0x000000, 1.0);
    renderer.setPixelRatio(window.devicePixelRatio);
    document.getElementById("view").appendChild(renderer.domElement);

    width = document.getElementById('view').clientWidth;
    height = document.getElementById('view').clientHeight;
    renderer.setSize(width, height);

    controls.target.set(427, 269, 77);

    






    var reader = new FileReader();
    reader.onload = (function (theFile) {
        return function (e) {
            var ifds = UTIF.decode(e.target.result);
            console.log(ifds.length);
            UTIF.decodeImages(e.target.result, ifds);
           // console.log(ifds);

            bbox = new THREE.BoxGeometry(ifds[0].height, ifds[0].width, ifds.length);
            console.log(ifds[0].height, ifds[0].width, ifds.length);
            var edges = new THREE.EdgesGeometry(bbox);
            var line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0xffffff }));
            line.position.set(Math.ceil(ifds[0].height / 2), Math.ceil(ifds[0].width / 2), Math.ceil(ifds.length/ 2));
            scene.add(line);
            controls.target.set(Math.ceil(ifds[0].height / 2), Math.ceil(ifds[0].width / 2), Math.ceil(ifds.length / 2));


            camera.position.x = ifds[0].height;
            camera.position.y = ifds[0].width+20;
            camera.position.z = ifds.length+400;
            //camera.lookat(Math.ceil(ifds[0].height / 2), Math.ceil(ifds[0].width / 2), Math.ceil(ifds.length / 2));


            for (j = 0; j < ifds.length; j++) {
                

                var rgba = UTIF.toRGBA8(ifds[j]);  // Uint8Array with RGBA pixels
              //  console.log(ifds[j].width, ifds[j].height, ifds[0]);
               
               
                var k; var contrast = 1; var thresh = 0;
                for (k = 0; k < rgba.byteLength; k = k + 4) {
                    if (rgba[k] < thresh) { rgba[k + 3] = 0; }
                    rgba[k] *= contrast;
                    rgba[k + 1] *= contrast;
                    rgba[k + 2] *= contrast;
                    if (rgba[k] == 0 && rgba[k + 1] == 0 && rgba[k + 1] == 0)
                        rgba[k + 3] = 0;
                   
                }
                //console.log(rgba);


                const gl = this.gl;



                // A begin
                var geometry = new THREE.Geometry(); //创建一个空几何体对象
                /**顶点坐标(纹理映射位置)*/


                var p1 = new THREE.Vector3(0 , 0 , j); //顶点1坐标
                var p2 = new THREE.Vector3(0 ,ifds[j].width, j ); //顶点2坐标
                var p3 = new THREE.Vector3(ifds[j].height, ifds[j].width,j); //顶点3坐标
                var p4 = new THREE.Vector3(ifds[j].height , 0 , j); //顶点4坐标
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
                var texture = new THREE.DataTexture(rgba, ifds[j].width, ifds[j].height, THREE.RGBAFormat);
                texture.needsUpdate = true;
                //console.log(texture);
                var material = new THREE.MeshBasicMaterial({ map: texture, side: THREE.DoubleSide });
                material.transparent = true;
                //material.opacity = 0.5;
                material.blending = THREE["CustomBlending"];
                material.blendEquation = THREE.MaxEquation;
                //material.depthWrite = false,
                //material.depthTest = false,
                //material.alphaTest = 0.5;

                var mesh = new THREE.Mesh(geometry, material);

                scene.add(mesh);
                render();

            }






           
        };
    })(file);
    reader.readAsArrayBuffer(file);
}


function readlocalfile() {
    document.getElementById('vaa3dweb-input-open-local').addEventListener('change', function (event) {
   show(event.target.files[0]);
       //show(event.target.files[0]);
    
        //console.log(event.target.files[0]);
   });
}