function stackdata(ifds) {
    console.log(ifds[0].width, ifds[0].height, ifds.lengths);
    var rgba = new Uint8Array(ifds[0].width * ifds[0].height * ifds.length * 4);
    for (j = 0; j < ifds.length; j++) {


        var data = UTIF.toRGBA8(ifds[j]);
        rgba.set(data, ifds[0].width * ifds[0].height * 4 * j);


        


    }
    return rgba;
}

function rearrangedata(data, width, height, depth,type)
{
   var rgba = new Uint8Array(data.length);

    if (type == 2) {
        for (i = 0; i < depth; i++) {
            for (j = 0; j < height; j++)
                for (k = 0; k < width; k++) {
                    rgba[i * width * height * 4 + j * width * 4 + k * 4] = data[i * 4 + j * depth * 4 + k * depth * height * 4];
                    rgba[i * width * height * 4 + j * width * 4 + k * 4 + 1] = data[i * 4 + j * depth * 4 + k * depth * height * 4 + 1];
                    rgba[i * width * height * 4 + j * width * 4 + k * 4 + 2] = data[i * 4 + j * depth * 4 + k * depth * height * 4 + 2];
                    rgba[i * width * height * 4 + j * width * 4 + k * 4 + 3] = data[i * 4 + j * depth * 4 + k * depth * height * 4 + 3];

                }

        }
    }

    if (type == 3) {
        for (i = 0; i < depth; i++) {
            for (j = 0; j < height; j++)
                for (k = 0; k < width; k++) {
                    rgba[i * width * height * 4 + j * width * 4 + k * 4] = data[i * 4*height + j  * 4 + k * depth * height * 4];
                    rgba[i * width * height * 4 + j * width * 4 + k * 4 + 1] = data[i * 4 * height + j  * 4 + k * depth * height * 4 + 1];
                    rgba[i * width * height * 4 + j * width * 4 + k * 4 + 2] = data[i * 4 * height + j  * 4 + k * depth * height * 4 + 2];
                    rgba[i * width * height * 4 + j * width * 4 + k * 4 + 3] = data[i * 4 * height + j  * 4 + k * depth * height * 4 + 3];

                }

        }
    }

    return rgba;


}

function renderimage(rgba, width, height, depth,scene,type) {
    for (j = 0; j < depth; j++) {



        // A begin
        var geometry = new THREE.Geometry(); //创建一个空几何体对象
        /**顶点坐标(纹理映射位置)*/




        if (type == 1) {
            var p1 = new THREE.Vector3(0, 0, j); //顶点1坐标
            var p2 = new THREE.Vector3(width, 0, j); //顶点2坐标
            var p3 = new THREE.Vector3(width, height, j); //顶点3坐标
            var p4 = new THREE.Vector3(0, height, j); //顶点4坐标
            geometry.vertices.push(p1, p2, p3, p4); //顶点坐标添加到geometry对象
            //console.log(p1, p2, p3, p4);
        }

        if (type == 2) {
            var p1 = new THREE.Vector3(j, 0, 0); //顶点1坐标
            var p2 = new THREE.Vector3(j, 0, width); //顶点2坐标
            var p3 = new THREE.Vector3(j,height,width); //顶点3坐标
            var p4 = new THREE.Vector3(j, height, 0); //顶点4坐标
            geometry.vertices.push(p1, p2, p3, p4); //顶点坐标添加到geometry对象
            //console.log(p1, p2, p3, p4);
        }

        if (type == 3) {
            var p1 = new THREE.Vector3(0, j, 0); //顶点1坐标
            var p2 = new THREE.Vector3(0, j, width); //顶点2坐标
            var p3 = new THREE.Vector3(height, j, width); //顶点3坐标
            var p4 = new THREE.Vector3(height, j,0 ); //顶点4坐标
            geometry.vertices.push(p1, p2, p3, p4); //顶点坐标添加到geometry对象
            //console.log(p1, p2, p3, p4);
        }

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
        var texture = new THREE.DataTexture(rgba.slice(j * width * height * 4, (j + 1) * width * height * 4), width, height, THREE.RGBAFormat);
        texture.needsUpdate = true;
        //console.log(texture);
        var material = new THREE.MeshBasicMaterial({ map: texture, side: THREE.DoubleSide });
        material.transparent = true;
        //material.opacity = 0.5;
        material.blending = THREE["CustomBlending"];
        material.blendEquation = THREE.MaxEquation;
        material.depthWrite = false;
        //material.depthTest = false,
        //material.alphaTest = 0.5;

        var mesh = new THREE.Mesh(geometry, material);



        scene.add(mesh);
        
       // console.log(j);
    }




}