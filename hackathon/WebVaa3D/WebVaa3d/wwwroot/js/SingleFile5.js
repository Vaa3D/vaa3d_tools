
function readlocalfile() {
   
    document.getElementById('vaa3dweb-input-open-local').addEventListener('change', function (event) {
        init(event.target.files[0]);
        //show(event.target.files[0]);

        //console.log(event.target.files[0]);
    });





    var container,
        renderer,
        scene,
        camera,
        controls,
        material,
        volconfig,
        cmtextures;


    function init(file) {

        scene = new THREE.Scene();

        // Create renderer
       
        renderer = new THREE.WebGLRenderer({ alpha: false });
        renderer.setClearColor(0x000000, 1.0);
        renderer.setPixelRatio(window.devicePixelRatio);
        width = document.getElementById('view').clientWidth;
        height = document.getElementById('view').clientHeight;
        renderer.setSize(width, height);
        document.getElementById("view").appendChild(renderer.domElement);

        //var h = 512; // frustum height
        //var aspect = width / height;
        ////camera = new THREE.OrthographicCamera(- h * aspect / 2, h * aspect / 2, h / 2, - h / 2, 1, 1000);
        ////camera.position.set(0, 0, 128);
        ////camera.up.set(0, 0, 1); // In our data, z is up

        camera = new THREE.PerspectiveCamera(45, width / height, 1, 10000);
      
        //camera.up.set(0, 0, 1); // In our data, z is up


        // Create controls
        controls = new THREE.OrbitControls(camera, document.getElementById('view'));

        controls.addEventListener('change', render);
        controls.target.set(64, 64, 128);
        //controls.minZoom = 0.5;
        //controls.maxZoom = 4;
        //controls.update();

        var axesHelper = new THREE.AxesHelper(1000);
        scene.add(axesHelper);

        // scene.add( new THREE.AxesHelper( 128 ) );

        // Lighting is baked into the shader a.t.m.
        // var dirLight = new THREE.DirectionalLight( 0xffffff );

        // The gui for interaction
        volconfig = { clim1: 0, clim2: 1, renderstyle: 'mip', isothreshold: 0.15, colormap: 'gray' };


        // Load the data ...
        var reader = new FileReader();
        reader.onload = (function (theFile) {
            return function (e) {
                var ifds = UTIF.decode(e.target.result);
                //console.log(ifds.length);
                UTIF.decodeImages(e.target.result, ifds);


                bbox = new THREE.BoxGeometry(ifds[0].width, ifds[0].height, ifds.length);
                //console.log(ifds[0].height, ifds[0].width, ifds.length);
                var edges = new THREE.EdgesGeometry(bbox);
                var line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0xffffff }));
                line.position.set(Math.ceil(ifds[0].width / 2), Math.ceil(ifds[0].height / 2), Math.ceil(ifds.length / 2));
                scene.add(line);
                controls.target.set(Math.ceil(ifds[0].width / 2), Math.ceil(ifds[0].height / 2), Math.ceil(ifds.length / 2));



                //console.log(ifds);
                rgba1 = stackdata(ifds);
                //console.log(rgba);
                rgba2 = rearrangedata(rgba1, ifds.length, ifds[0].height, ifds[0].width,2);
                //console.log(rgba2);
                rgba3 = rearrangedata(rgba1, ifds.length,ifds[0].width,  ifds[0].height,3);
                renderimage(rgba1, ifds[0].width, ifds[0].height, ifds.length,scene,1);
                renderimage(rgba2, ifds.length, ifds[0].height, ifds[0].width,scene,2);
              renderimage(rgba3, ifds.length, ifds[0].width,  ifds[0].height,scene,3);

                console.log(scene);


                camera.position.x = ifds[0].height;
                camera.position.y = ifds[0].width + 20;
                camera.position.z = ifds.length + 400;



                render();
             
            };
        })(file);
        reader.readAsArrayBuffer(file);


        function render() {

            renderer.render(scene, camera);

        }


    }
}

function mergeTypedArraysUnsafe(a, b) {
    var c = new a.constructor(a.length + b.length);
    c.set(a);
    c.set(b, a.length);

    return c;
}
