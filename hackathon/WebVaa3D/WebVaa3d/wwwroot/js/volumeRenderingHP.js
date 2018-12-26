function VolumeRender() {
    if (WEBGL.isWebGL2Available() === false) {

        document.body.appendChild(WEBGL.getWebGL2ErrorMessage());

    }
    document.getElementById('vaa3dweb-input-open-local').addEventListener('change', function (event) {
        //handleFileSelect();
        if (window.File && window.FileReader && window.FileList && window.Blob) {
            // Great success! All the File APIs are supported.
            var files = event.target.files;
            for (var i = 0, f; f = files[i]; i++) {
                if (!f.type.match('tif.*')) {//只允许打开tif格式的文件
                    console.log('only open tif file!');
                    continue;
                }
                console.log(f.name);
                init(f,i);//目前考虑一张张图片渲染，计算图片位置？？？
            }
            //init(event.target.files[0]);//初始化打开单个tif文件
            
        } else {
            alert('The File APIs are not fully supported in this browser.');
        }
        
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
    var length, width, height;//大小先默认了一个值，后期确定整体图片大小再更改------------------------？？

    function init(file, level) {
        var filesname = file.name;
        var regex = /\d+/g;
        var filestr = filesname.match(regex);//将文件名显示的长宽高三个量分离到数组中去
        
        scene = new THREE.Scene();

        // Create renderer
        var canvas = document.createElement('canvas');//create canvas
        var context = canvas.getContext('webgl2');//SET canvas
        renderer = new THREE.WebGLRenderer({ canvas: canvas, context: context });
        renderer.setClearColor(0x000000, 1.0);
        renderer.setPixelRatio(window.devicePixelRatio);//兼容高清屏幕
        width = document.getElementById('view').clientWidth;
        height = document.getElementById('view').clientHeight;
        renderer.setSize(width, height);
        document.getElementById("view").appendChild(renderer.domElement);//在view中添加canvas

        var h = 512; // frustum height
        var aspect = width / height;

        //camera = new THREE.OrthographicCamera(- h * aspect / 2, h * aspect / 2, h / 2, - h / 2, 1, 1000);
        //camera.position.set(0, 0, 128);
        //camera.up.set(0, 0, 1); // In our data, z is up

        camera = new THREE.PerspectiveCamera(45, width / height, 1, 10000);//远景相机（透视相机）以人眼的形式观察的，（观察角度，宽高比，近裁剪切面，远裁剪切面）

        camera.position.set(0, 0, 128);//我们调用 scene.add() 时，对象将被添加到原点处，即坐标点(0,0,0)，这将导致相机和立方体发生空间重叠。为了避免这样，我们把相机（camera）的位置移出来一些。

        camera.up.set(0, 0, 1); // In our data, z is up 防止和画布重叠？---


        // Create controls
        controls = new THREE.OrbitControls(camera, document.getElementById('view'));//轨道控制允许摄像机围绕目标进行轨道运行，进行旋转

        controls.addEventListener('change', render);
        controls.target.set(64, 64, 128);//大小怎么设置的？----------
        //controls.minZoom = 0.5;
        //controls.maxZoom = 4;
        //controls.update();

        // scene.add( new THREE.AxesHelper( 128 ) );

        // Lighting is baked into the shader a.t.m.
        // var dirLight = new THREE.DirectionalLight( 0xffffff );

        // The gui for interaction
        volconfig = { clim1: 0, clim2: 1, renderstyle: 'mip', isothreshold: 0.15, colormap: 'gray' };


        // Load the data ...
        var reader = new FileReader();//读取本地文件
        reader.readAsArrayBuffer(file);
        reader.onload = (function (theFile) {
            return function (e) {
                var ifds = UTIF.decode(e.target.result);//解码tif文件，返回“IFD”数组（图像文件目录）
                console.log(ifds.length);
                UTIF.decodeImages(e.target.result, ifds);//循环通过每个IFD，并添加三个新的属性：width,length,height   
                //console.log(ifds);


                //将图片添加到场景中去
                var rgba = new Float32Array();
                rgba = UTIF.toRGBA8(ifds[0]);//以RGBA格式返回图像的Uint8Array，每个通道8位

                for (j = 1; j < ifds.length; j++) {//读取多个文件？------
                    var data = UTIF.toRGBA8(ifds[j]);
     
                    rgba = mergeTypedArraysUnsafe(rgba, data);
                   
                }
                console.log(rgba);
                //var rgba2 = new Float32Array(rgba.length / 4);     
                //for (j = 0; j < rgba.length / 4; j++) {
                //    rgba2[j] = rgba[j * 4]/255;
                //}
                //console.log(rgba2);

                var texture = new THREE.DataTexture3D(rgba, ifds[0].width, ifds[0].height, ifds.length);//直接从原始数据和宽高来创建纹理。
               //texture.format = THREE.RedFormat;
                texture.format = THREE.RGBAFormat;//如果format为THREE.RGBAFormat, 那么data里面每4个值代表一个纹理单元（texel）; Red, Green, Blue 和 Alpha (透明度)。
                texture.type = THREE.UnsignedByteType;//THREE.UnsignedByteType, a Uint8Array will be useful for addressing the texel data.
                texture.minFilter = texture.magFilter = THREE.LinearFilter;
                texture.unpackAlignment = 1;
                texture.needsUpdate = true;
                console.log(texture);
                // Colormap textures
                

                cmtextures = {
                   // viridis: new THREE.TextureLoader().load('textures/cm_viridis.png', render),
                   // gray: new THREE.TextureLoader().load('textures/cm_gray.png', render),
                    
                };

               // console.log(cmtextures);

                // Material
                var shader = THREE.VolumeRenderShader1;//自定义阴影

                var uniforms = THREE.UniformsUtils.clone(shader.uniforms);//支持统一变量的合并和克隆

                uniforms.u_data.value = texture;
                uniforms.u_size.value.set( ifds[0].width,ifds[0].height, ifds.length);
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
                console.log(filestr[0] + "" + filestr[1] + "" + filestr[2]);
                geometry.translate(0 +( filestr[0] / 136960 * ifds[0].height)/2-0.5, 0 + (filestr[1] / 114560 * ifds[0].width/2)-0.5, 0 + (filestr[2] / 49280 * ifds.length)/2-0.5);

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

       
        function helper() {
           //大小先默认了一个值，后期确定整体图片大小再更改------------------------？？
            length = 853;
            width = 537;
            height = 153;
            var axesHelper = new THREE.AxesHelper(1000);//three.js轴辅助功能，显示坐标轴,参数：坐标轴的大小
            scene.add(axesHelper);

            //将模型的边框显示出来添加到场景中去,是否可以用edges.helper替换这个？？？
            bbox = new THREE.BoxGeometry(length, width, height);//盒子模型，添加的物体
            //console.log(ifds[0].height, ifds[0].width, ifds.length);
            var edges = new THREE.EdgesGeometry(bbox);//这可以用作辅助对象来查看Geometry对象的边缘。
            var line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0xffffff }));//一种绘制线框式结构的材料。
            line.position.set(Math.ceil(length / 2), Math.ceil(width / 2), Math.ceil(height / 2));
            scene.add(line);
            controls.target.set(Math.ceil(length / 2), Math.ceil(width / 2), Math.ceil(height / 2)); //math.ceil是向上取整函数

        }

        function render() {
            helper();
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
