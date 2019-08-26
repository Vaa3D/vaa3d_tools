var light, pointLight, ambientLight;


function show(file) {
    renderer = new THREE.WebGLRenderer({ alpha: false });
    width = document.getElementById('view').clientWidth;
    height = document.getElementById('view').clientHeight;
    renderer.setSize(width, height);
    camera = new THREE.PerspectiveCamera(45, width / height, 1, 10000);
    scene = new THREE.Scene;
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

    //materials = generateMaterials();
    //current_material = "shiny";

    // MARCHING CUBES

    resolution = 28;
    numBlobs = 10;


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
            line.position.set(Math.ceil(ifds[0].height / 2), Math.ceil(ifds[0].width / 2), Math.ceil(ifds.length / 2));
            scene.add(line);
            controls.target.set(Math.ceil(ifds[0].height / 2), Math.ceil(ifds[0].width / 2), Math.ceil(ifds.length / 2));


            camera.position.x = ifds[0].height;
            camera.position.y = ifds[0].width + 20;
            camera.position.z = ifds.length + 400;
            //camera.lookat(Math.ceil(ifds[0].height / 2), Math.ceil(ifds[0].width / 2), Math.ceil(ifds.length / 2));


            var rgba = new Array();
            rgba = UTIF.toRGBA8(ifds[0]);
            //console.log(rgba);
            for (j = 1; j < ifds.length; j++) {
                var data = UTIF.toRGBA8(ifds[j]);
                rgba = mergeTypedArraysUnsafe(rgba, data);

            }
            // console.log(rgba);

            var texture = new THREE.DataTexture3D(rgba, ifds[0].height, ifds[0].width, ifds.length);
            
            texture.format = THREE.RGBAFormat;
            texture.type = THREE.UnsignedByteType;
            //texture.minFilter = texture.magFilter = THREE.LinearFilter;
            texture.unpackAlignment = 1;
            texture.needsUpdate = true;
            console.log(texture);
                // Colormap textures
            var material = new THREE.MeshBasicMaterial({ map: texture, side: THREE.BackSide });
          
            material.transparent = true;
            //material.opacity = 0.5;
            material.blending = THREE["CustomBlending"];
            material.blendEquation = THREE.MaxEquation;
            //material.depthWrite = false,
            //material.depthTest = false,
            //material.alphaTest = 0.5;

            var mesh = new THREE.Mesh(bbox, material);
            console.log(texture);



            scene.add(mesh);
            render();
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

