const axios = window.axios;
renderer = new THREE.WebGLRenderer({ antialias: false });
camera = new THREE.PerspectiveCamera();
scene = new THREE.Scene();
controls = new THREE.OrbitControls(camera);//创建控件对象

function render() {
    //requestAnimationFrame(render);
    renderer.render(scene, camera);

}
//console.log(scene.children);
//scene.children.material.blending = THREE["CustomBlending"];
//scene.children.material.blendEquation = THREE.MaxEquation;
render();


async function getPictures()
{
    var id = 0;
    const res = await axios.get(`api/tiff/${+id}`);
    width = document.getElementById('view').clientWidth;
    height = document.getElementById('view').clientHeight;
    renderer.setSize(width, height);
    camera = new THREE.PerspectiveCamera(45, width / height, 1, 10000);

    var files = res.data;
    //console.log(files);
   
   
    document.getElementById("view").appendChild(renderer.domElement);
   
    camera.position.x = 567;
    camera.position.y = 562;
    camera.position.z = -659;

    
    renderer.setClearColor(0x000000, 1.0);
    renderer.setPixelRatio(window.devicePixelRatio);


    for (index in files)
    //for (i = 0; i <= 5;i++ )
    {   //index=i;
        const file = files[index];
        const response = await axios.get(`api/tiff/${+id}/${file}`, { responseType: "arraybuffer" });


        var fileName = $("#file").val();
        console.log(file);
        var regex = /\d+/g;
        var filestr = file.match(regex);

        var tiff = new Tiff({ buffer: response.data });
        var count = tiff.countDirectory();






        //console.log(count);
        var j;
        for (j = 0; j < count; j++) {
            tiff.setDirectory(j);
            var buffer = tiff.readRGBAImage();
            //console.log(tiff.width(),tiff.height());
            const rgba = new Uint8Array(buffer);

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



            const gl = this.gl;



            // A begin
            var geometry = new THREE.Geometry(); //创建一个空几何体对象
            /**顶点坐标(纹理映射位置)*/


            var p1 = new THREE.Vector3(0 + filestr[0] / 136960 * tiff.height(), 0 + filestr[1] / 114560 * tiff.width(), j + filestr[2] / 49280 * count); //顶点1坐标
            var p2 = new THREE.Vector3(0 + filestr[0] / 136960 * tiff.height(), tiff.width() + filestr[1] / 114560 * tiff.width(), j + filestr[2] / 49280 * count); //顶点2坐标
            var p3 = new THREE.Vector3(tiff.height() + filestr[0] / 136960 * tiff.height(), tiff.width() + filestr[1] / 114560 * tiff.width(), j + filestr[2] / 49280 * count); //顶点3坐标
            var p4 = new THREE.Vector3(tiff.height() + filestr[0] / 136960 * tiff.height(), 0 + filestr[1] / 114560 * tiff.width(), j + filestr[2] / 49280 * count); //顶点4坐标
            geometry.vertices.push(p1, p2, p3, p4); //顶点坐标添加到geometry对象



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
            var texture = new THREE.DataTexture(rgba, tiff.width(), tiff.height(), THREE.RGBAFormat);
            texture.needsUpdate = true;
            //console.log(texture);
            var material = new THREE.MeshBasicMaterial({ map: texture, side: THREE.DoubleSide });
            material.transparent = true;
            //material.opacity = 0.5;
            material.blending = THREE["CustomBlending"];
            material.blendEquation = THREE.MaxEquation;
            //material.depthWrite = false,
            //material.depthTest = false,
            material.alphaTest = 0.5;

            var mesh = new THREE.Mesh(geometry, material);






            scene.add(mesh);


        }
        tiff.close();


    }




    var raycaster = new THREE.Raycaster();
    var mouse = new THREE.Vector2();

    level = 0;

    tiff_width = [179, 215, 239, 253, 253];
    tiff_height = [213, 244, 244, 253, 253];
    tiff_depth = [153, 154, 205, 246, 247];
    unit_width = [114560, 68800, 38240, 20240, 10120];
    unit_height = [136960, 78080, 39040, 20240, 10120];
    unit_depth = [98560, 49280, 32800, 19680, 9840];

    async function onMouseClick(event) {
        console.log(camera.position);

        //通过鼠标点击的位置计算出raycaster所需要的点的位置，以屏幕中心为原点，值的范围为-1到1.

        mouse.x = (event.clientX / width) * 2 - 1;
        mouse.y = - (event.clientY / height) * 2 + 1;

        // 通过鼠标点的位置和当前相机的矩阵计算出raycaster
        raycaster.setFromCamera(mouse, camera);

        // 获取raycaster直线和所有模型相交的数组集合
        var intersects = raycaster.intersectObjects(scene.children);

        var intensity = new Array();
        for (var i = 0; i < intersects.length; i++) {

            intensity[i] = intersects[i].object.material.map.image.data[(parseInt(intersects[i].point.y % tiff_width[level]) - 1) * tiff_height[level] + parseInt(intersects[i].point.x % tiff_height[level])];

        }
        console.log(intensity);
        var indexOfMaxintensity = intensity.reduce((iMax, x, i, arr) => x > arr[iMax] ? i : iMax, 0);
        console.log(indexOfMaxintensity);

        //console.log(parseInt(intersects[0].point.x % 213), parseInt(intersects[0].point.y % 179));
        //console.log((parseInt(intersects[0].point.y % tiff_width[level]) - 1) * tiff_height[level] + parseInt(intersects[0].point.x % tiff_height[level]));
        //console.log(intersects[0].object.material.map.image.data[(parseInt(intersects[0].point.y % tiff_width[level])-1) * tiff_height[level] + parseInt(intersects[0].point.x % tiff_height[level])]);

        zoomin_width = Math.floor(intersects[indexOfMaxintensity].point.x / tiff_height[level] * unit_height[level] / unit_height[level + 1]) * unit_height[level + 1];
        zoomin_height = Math.floor(intersects[indexOfMaxintensity].point.y / tiff_width[level] * unit_width[level] / unit_width[level + 1]) * unit_width[level + 1];
        zoomin_depth = Math.floor(intersects[indexOfMaxintensity].point.z / tiff_depth[level] * unit_depth[level] / unit_depth[level + 1]) * unit_depth[level + 1];
        console.log(level);
        console.log(zoomin_width, zoomin_height, zoomin_depth);

        while (scene.children.length > 0) { scene.remove(scene.children[0]); }
        const re = await axios.get(`api/tiff/${+id + level + 1}`)


        var files = re.data;

        for (index in files) {
            const file = files[index];
            var fileName = $("#file").val();

            var regex = /\d+/g;
            var filestr = file.match(regex);

            if (parseInt(filestr[0] / unit_height[level + 1]) * unit_height[level + 1] == zoomin_width && parseInt(filestr[1] / unit_width[level + 1]) * unit_width[level + 1] == zoomin_height && parseInt(filestr[2] / unit_depth[level + 1]) * unit_depth[level + 1] == zoomin_depth) {
                //if ((filestr[0] <= zoomin_width + unit_height[level + 1]) && (filestr[0] >= zoomin_width - unit_height[level + 1]) && (filestr[1] <= zoomin_height + unit_width[level + 1]) && (filestr[1] >= zoomin_height - unit_width[level + 1]) && (filestr[2] <= zoomin_depth) && (filestr[2] >= zoomin_depth - unit_depth[level + 1])) {
                // if ((filestr[0] <= zoomin_width ) && (filestr[0] >= zoomin_width - unit_height[level + 1]) && (filestr[1] <= zoomin_height ) && (filestr[1] >= zoomin_height - unit_width[level + 1]) && (filestr[2] <= zoomin_depth) && (filestr[2] >= zoomin_depth - unit_depth[level + 1])) {
                const response = await axios.get(`api/tiff/${+id + level + 1}/${file}`, { responseType: "arraybuffer" })
                console.log(file);


                var tiff = new Tiff({ buffer: response.data });
                var count = tiff.countDirectory();

                var j;
                for (j = 0; j < count; j++) {
                    tiff.setDirectory(j);
                    var buffer = tiff.readRGBAImage();
                    //console.log(tiff.width(),tiff.height());
                    const rgba = new Uint8Array(buffer);

                    var k; var contrast = 12; var thresh = 2;
                    for (k = 0; k < rgba.byteLength; k = k + 4) {
                        if (rgba[k] < thresh) { rgba[k + 3] = 0; }
                        rgba[k] *= contrast;
                        rgba[k + 1] *= contrast;
                        rgba[k + 2] *= contrast;
                        if (rgba[k] == 0 && rgba[k + 1] == 0 && rgba[k + 1] == 0)
                            rgba[k + 3] = 0;

                    }

                    const gl = this.gl;



                    // A begin
                    var geometry = new THREE.Geometry(); //创建一个空几何体对象
                    /**顶点坐标(纹理映射位置)*/


                    var p1 = new THREE.Vector3(0 + filestr[0] / unit_height[level + 1] * tiff_height[level + 1], 0 + filestr[1] / unit_width[level + 1] * tiff_width[level + 1], j + filestr[2] / unit_depth[level + 1] * count); //顶点1坐标
                    var p2 = new THREE.Vector3(0 + filestr[0] / unit_height[level + 1] * tiff_height[level + 1], tiff_width[level + 1] + filestr[1] / unit_width[level + 1] * tiff_width[level + 1], j + filestr[2] / unit_depth[level + 1] * count); //顶点2坐标
                    var p3 = new THREE.Vector3(tiff_height[level + 1] + filestr[0] / unit_height[level + 1] * tiff_height[level + 1], tiff_width[level + 1] + filestr[1] / unit_width[level + 1] * tiff_width[level + 1], j + filestr[2] / unit_depth[level + 1] * count); //顶点3坐标
                    var p4 = new THREE.Vector3(tiff_height[level + 1] + filestr[0] / unit_height[level + 1] * tiff_height[level + 1], 0 + filestr[1] / unit_width[level + 1] * tiff_width[level + 1], j + filestr[2] / unit_depth[level + 1] * count); //顶点4坐标
                    geometry.vertices.push(p1, p2, p3, p4); //顶点坐标添加到geometry对象
                    //console.log(filestr[0], filestr[1]);




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
                    var texture = new THREE.DataTexture(rgba, tiff.width(), tiff.height(), THREE.RGBAFormat);

                    texture.needsUpdate = true;
                    //console.log(texture);
                    var material = new THREE.MeshBasicMaterial({ map: texture, side: THREE.DoubleSide});
                    material.transparent = true;
                    //material.blending = THREE[ "MultiplyBlending"];
                    //material.depthWrite = false,
                    //material.depthTest = false,
                    material.blending = THREE["CustomBlending"];
                    material.blendEquation = THREE.MaxEquation;
                    material.alphaTest = 0.5;

                    var mesh = new THREE.Mesh(geometry, material);






                    scene.add(mesh);

                }
                tiff.close();
                render();

            }
        }

        level = level + 1;
        //console.log(camera.position);
        console.log(p1, p2, p3, p4);
        camera.position.x = 0;
        camera.position.y = 0;
        camera.position.z = 0;
        camera.position.addVectors(p1, p3);
        camera.position.divideScalar(2);
        camera.lookAt(camera.position);
        console.log(camera.position);
        render();
    }

    
    // 添加window 的resize事件监听
    window.addEventListener('resize', onWindowResize);
    function onWindowResize() {
        width = document.getElementById('view').clientWidth;
        height = document.getElementById('view').clientHeight;
        // 重新设置相机宽高比例
        camera.aspect = width / height;
        // 更新相机投影矩阵
        camera.updateProjectionMatrix();
        // 重新设置渲染器渲染范围
        renderer.setSize(width, height);
        setTimeout(renderer.render(scene, camera), 300);
    }



    window.addEventListener('dblclick', onMouseClick, false);
   // window.addEventListener('resize', onWindowResize, false);
    controls = new THREE.OrbitControls(camera, document.getElementById('view'));//创建控件对象
    controls.addEventListener('change', render);//监听鼠标、键盘事件
    document.getElementById('left-content').addEventListener('fullscreenchange', render);




    // handle success
    // this.imgs.push(new Tiff({ buffer: response.data }))



}

function draw()
{   
    function VF(event) {
        controls.removeEventListener('change', render);
        points = new Array();
        var raycaster = new THREE.Raycaster();
        var mouse = new THREE.Vector2();
        raycaster.setFromCamera(mouse, camera);
        var intersects = raycaster.intersectObjects(scene.children);
        console.log(points);
        window.addEventListener('mouseup', controls.addEventListener('change', render), false);
       
    }
    window.addEventListener('mousedown', VF, false);
    
}