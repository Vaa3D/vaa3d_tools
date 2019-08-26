
function VolumeRender() {
    if (WEBGL.isWebGL2Available() === false) {

        document.body.appendChild(WEBGL.getWebGL2ErrorMessage());

    }
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
        var canvas = document.createElement('canvas');
        var context = canvas.getContext('webgl2');
        renderer = new THREE.WebGLRenderer({ canvas: canvas, context: context });
        renderer.setClearColor(0x000000, 1.0);
        renderer.setPixelRatio(window.devicePixelRatio);
        width = document.getElementById('view').clientWidth;
        height = document.getElementById('view').clientHeight;
        renderer.setSize(width, height);
        document.getElementById("view").appendChild(renderer.domElement);

        var h = 512; // frustum height
        var aspect = width / height;
        camera = new THREE.OrthographicCamera(- h * aspect / 2, h * aspect / 2, h / 2, - h / 2, 1, 1000);
        camera.position.set(0, 0, 128);
        camera.up.set(0, 0, 1); // In our data, z is up

        //camera = new THREE.PerspectiveCamera(45, width / height, 1, 10000);

        //camera.position.set(0, 0, 128);

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
                var rgba = new Uint8Array(UTIF.toRGBA8(ifds[0]).length * ifds.length);
                //console.log(rgba);
                //rgba = UTIF.toRGBA8(ifds[0]);
                var data = 0;
                for (j = 0; j < ifds.length; j++) {
                  data = UTIF.toRGBA8(ifds[j]);
                    rgba.set(data, j * UTIF.toRGBA8(ifds[0]).length);
                    //console.log(data);
                }
                //console.log(rgba);
                //var rgba2 = new Float32Array(rgba.length / 4);


                //for (j = 0; j < rgba.length / 4; j++) {
                //    rgba2[j] = rgba[j * 4]/255;
                //}
                //console.log(rgba2);

                var texture = new THREE.DataTexture3D(rgba, ifds[0].width, ifds[0].height, ifds.length);
                //texture.format = THREE.RedFormat;
                texture.format = THREE.RGBAFormat;
                texture.type = THREE.UnsignedByteType;
                texture.minFilter = texture.magFilter = THREE.LinearFilter;
                texture.unpackAlignment = 1;
                texture.needsUpdate = true;
                console.log(texture);
                // Colormap textures


                cmtextures = {
                    viridis: new THREE.TextureLoader().load('textures/cm_viridis.png', render),
                    gray: new THREE.TextureLoader().load('textures/cm_gray.png', render),

                };

                // console.log(cmtextures);

                // Material
                var shader = THREE.VolumeRenderShader2;

                var uniforms = THREE.UniformsUtils.clone(shader.uniforms);

                uniforms.u_data.value = texture;
                uniforms.u_size.value.set(ifds[0].width, ifds[0].height, ifds.length);
                uniforms.u_clim.value.set(volconfig.clim1, volconfig.clim2);
                uniforms.u_renderstyle.value = volconfig.renderstyle == 'mip' ? 0 : 1; // 0: MIP, 1: ISO
                uniforms.u_renderthreshold.value = volconfig.isothreshold; // For ISO renderstyle
                uniforms.u_cmdata.value = cmtextures[volconfig.colormap];

                material = new THREE.ShaderMaterial({
                    uniforms: uniforms,
                    vertexShader: shader.vertexShader,
                    fragmentShader: shader.fragmentShader,
                    side: THREE.BackSide // The volume shader uses the backface as its "reference point"
                });
                material.transparent = true;
                // Mesh
                var geometry = new THREE.BoxGeometry(ifds[0].width, ifds[0].height, ifds.length);
                geometry.translate(ifds[0].width / 2 - 0.5, ifds[0].height / 2 - 0.5, ifds.length / 2 - 0.5);

                var mesh = new THREE.Mesh(geometry, material);
                scene.add(mesh);

                render();

                console.log(mesh);



                // window.addEventListener('resize', onWindowResize, false);



                function updateUniforms() {

                    material.uniforms.u_clim.value.set(volconfig.clim1, volconfig.clim2);
                    material.uniforms.u_renderstyle.value = volconfig.renderstyle == 'mip' ? 0 : 1; // 0: MIP, 1: ISO
                    material.uniforms.u_renderthreshold.value = volconfig.isothreshold; // For ISO renderstyle
                    material.uniforms.u_cmdata.value = cmtextures[volconfig.colormap];

                    render();

                }

                //function onWindowResize() {

                //    renderer.setSize(window.innerWidth, window.innerHeight);

                //    var aspect = window.innerWidth / window.innerHeight;

                //    var frustumHeight = camera.top - camera.bottom;

                //    camera.left = - frustumHeight * aspect / 2;
                //    camera.right = frustumHeight * aspect / 2;

                //    camera.updateProjectionMatrix();

                //    render();

                //}


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
