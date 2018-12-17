/**
 * @author Almar Klein / http://almarklein.org
 *
 * Shaders to render 3D volumes using raycasting.
 * The applied techniques are based on similar implementations in the Visvis and Vispy projects.
 * This is not the only approach, therefore it's marked 1.
 */

THREE.VolumeRenderShader2 = {
    uniforms: {
        "u_size": { value: new THREE.Vector3(1, 1, 1) },
        "u_renderstyle": { value: 0 },
        "u_renderthreshold": { value: 0.5 },
        "u_clim": { value: new THREE.Vector2(1, 1) },
        "u_data": { value: null },
        "u_cmdata": { value: null }
    },
    vertexShader: [
        'varying vec4 v_nearpos;',
        'varying vec4 v_farpos;',
        'varying vec3 v_position;',

        'mat4 inversemat(mat4 m) {',
        // Taken from https://github.com/stackgl/glsl-inverse/blob/master/index.glsl
        // This function is licenced by the MIT license to Mikola Lysenko
        'float',
        'a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],',
        'a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],',
        'a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],',
        'a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],',

        'b00 = a00 * a11 - a01 * a10,',
        'b01 = a00 * a12 - a02 * a10,',
        'b02 = a00 * a13 - a03 * a10,',
        'b03 = a01 * a12 - a02 * a11,',
        'b04 = a01 * a13 - a03 * a11,',
        'b05 = a02 * a13 - a03 * a12,',
        'b06 = a20 * a31 - a21 * a30,',
        'b07 = a20 * a32 - a22 * a30,',
        'b08 = a20 * a33 - a23 * a30,',
        'b09 = a21 * a32 - a22 * a31,',
        'b10 = a21 * a33 - a23 * a31,',
        'b11 = a22 * a33 - a23 * a32,',

        'det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;',

        'return mat4(',
        'a11 * b11 - a12 * b10 + a13 * b09,',
        'a02 * b10 - a01 * b11 - a03 * b09,',
        'a31 * b05 - a32 * b04 + a33 * b03,',
        'a22 * b04 - a21 * b05 - a23 * b03,',
        'a12 * b08 - a10 * b11 - a13 * b07,',
        'a00 * b11 - a02 * b08 + a03 * b07,',
        'a32 * b02 - a30 * b05 - a33 * b01,',
        'a20 * b05 - a22 * b02 + a23 * b01,',
        'a10 * b10 - a11 * b08 + a13 * b06,',
        'a01 * b08 - a00 * b10 - a03 * b06,',
        'a30 * b04 - a31 * b02 + a33 * b00,',
        'a21 * b02 - a20 * b04 - a23 * b00,',
        'a11 * b07 - a10 * b09 - a12 * b06,',
        'a00 * b09 - a01 * b07 + a02 * b06,',
        'a31 * b01 - a30 * b03 - a32 * b00,',
        'a20 * b03 - a21 * b01 + a22 * b00) / det;',
        '}',


        'void main() {',
        // Prepare transforms to map to "camera view". See also:
        // https://threejs.org/docs/#api/renderers/webgl/WebGLProgram
        'mat4 viewtransformf = viewMatrix;',
        'mat4 viewtransformi = inversemat(viewMatrix);',

        // Project local vertex coordinate to camera position. Then do a step
        // backward (in cam coords) to the near clipping plane, and project back. Do
        // the same for the far clipping plane. This gives us all the information we
        // need to calculate the ray and truncate it to the viewing cone.
        'vec4 position4 = vec4(position, 1.0);',
        'vec4 pos_in_cam = viewtransformf * position4;',

        // Intersection of ray and near clipping plane (z = -1 in clip coords)
        'pos_in_cam.z = -pos_in_cam.w;',
        'v_nearpos = viewtransformi * pos_in_cam;',

        // Intersection of ray and far clipping plane (z = +1 in clip coords)
        'pos_in_cam.z = pos_in_cam.w;',
        'v_farpos = viewtransformi * pos_in_cam;',

        // Set varyings and output pos
        'v_position = position;',
        'gl_Position = projectionMatrix * viewMatrix * modelMatrix * position4;',
        '}',
    ].join('\n'),
    fragmentShader: [
        'precision highp float;',
        'precision mediump sampler2D;',

        'uniform vec3 u_size;',
        'uniform int u_renderstyle;',
        'uniform float u_renderthreshold;',
        'uniform vec2 u_clim;',

        'uniform sampler2D u_data[500];',
        'uniform sampler2D u_cmdata;',

        'varying vec3 v_position;',
        'varying vec4 v_nearpos;',
        'varying vec4 v_farpos;',

        // The maximum distance through our rendering volume is sqrt(3).
        'const int MAX_STEPS = 1774;  // 887 for 512^3, 1774 for 1024^3',
        'const int REFINEMENT_STEPS = 4;',
        'const float relative_step_size = 1.0;',

        'const vec4 ambient_color = vec4(0.2, 0.4, 0.2, 1.0);',
        'const vec4 diffuse_color = vec4(0.8, 0.2, 0.2, 1.0);',
        'const vec4 specular_color = vec4(1.0, 1.0, 1.0, 1.0);',

        //'const vec4 ambient_color = vec4(0.0, 0.0, 0.0, 1.0);',
        //'const vec4 diffuse_color = vec4(0.0, 0.0, 0.0, 1.0);',
        //'const vec4 specular_color = vec4(0.0, 0.0, 0.0, 1.0);',

        'const float shininess = 4.0;',

        'void cast_mip(vec3 start_loc, vec3 step, int nsteps, vec3 view_ray);',


        'vec3 sample2(vec3 texcoords);',
        'vec4 apply_colormap(float val);',
        'vec4 add_lighting(float val, vec3 loc, vec3 step, vec3 view_ray);',


        'void main() {',
        // Normalize clipping plane info
        'vec3 farpos = v_farpos.xyz / v_farpos.w;',
        'vec3 nearpos = v_nearpos.xyz / v_nearpos.w;',

        // Calculate unit vector pointing in the view direction through this fragment.
        'vec3 view_ray = normalize(nearpos.xyz - farpos.xyz);',

        // Compute the (negative) distance to the front surface or near clipping plane.
        // v_position is the back face of the cuboid, so the initial distance calculated in the dot
        // product below is the distance from near clip plane to the back of the cuboid
        'float distance = dot(nearpos - v_position, view_ray);',
        'distance = max(distance, min((-0.5 - v_position.x) / view_ray.x,',
        '(u_size.x - 0.5 - v_position.x) / view_ray.x));',
        'distance = max(distance, min((-0.5 - v_position.y) / view_ray.y,',
        '(u_size.y - 0.5 - v_position.y) / view_ray.y));',
        'distance = max(distance, min((-0.5 - v_position.z) / view_ray.z,',
        '(u_size.z - 0.5 - v_position.z) / view_ray.z));',

        // Now we have the starting position on the front surface
        'vec3 front = v_position + view_ray * distance;',

        // Decide how many steps to take
        'int nsteps = int(-distance / relative_step_size + 0.5);',
        'if ( nsteps < 1 )',
        'discard;',

        // Get starting location and step vector in texture coordinates
        'vec3 step = ((v_position - front) / u_size) / float(nsteps);',
        'vec3 start_loc = front / u_size;',

        // For testing: show the number of steps. This helps to establish
        // whether the rays are correctly oriented
        //'gl_FragColor = vec4(0.0, float(nsteps) / 1.0 / u_size.x, 1.0, 1.0);',
        //'return;',

        'if (u_renderstyle == 0)',
        //'gl_FragColor = vec4(sample2(v_position),1);',
        'cast_mip(start_loc, step, nsteps, view_ray);',
     

        'if (gl_FragColor.a < 0.05)',
        'discard;',
        '}',



        'vec3 sample2(vec3 texcoords) {',
        '/* Sample float value from a 3D texture. Assumes intensity data. */',
        'int index=int(texcoords.z);',
        'return texture2D(u_data[index], texcoords.xy).rgb;',
        '}',


        'vec4 apply_colormap(float val) {',
        'val = (val - u_clim[0]) / (u_clim[1] - u_clim[0]);',
        'return texture2D(u_cmdata, vec2(val, 0.5));',
        '}',


        'void cast_mip(vec3 start_loc, vec3 step, int nsteps, vec3 view_ray) {',

        'float max_val = -1e6;',

        'int max_i = 100;',
        'float alpha = 1.0;',
        'vec3 loc = start_loc;',
        'vec3 max_val2 = sample2(loc);',

        // Enter the raycasting loop. In WebGL 1 the loop index cannot be compared with
        // non-constant expression. So we use a hard-coded max, and an additional condition
        // inside the loop.
        'for (int iter=0; iter<MAX_STEPS; iter++) {',
        'if (iter >= nsteps)',
        'break;',
        // Sample from the 3D texture
        'float val = 0.21*sample2(loc).r+0.7152*sample2(loc).g+0.0722*sample2(loc).b;',
        // Apply MIP operation
        'if (val > max_val) {',
        'max_val = val;',
        'max_i = iter;',
        'max_val2 =sample2(loc);',
        '}',
        // Advance location deeper into the volume
        'loc += step;',
        '}',

        // Refine location, gives crispier images
        //'vec3 iloc = start_loc + step * (float(max_i) - 0.5);',
        //'vec3 istep = step / float(REFINEMENT_STEPS);',
        //'for (int i=0; i<REFINEMENT_STEPS; i++) {',
        //'max_val = max(max_val, sample2(iloc));',
        //'iloc += istep;',
        //'}',

        // Resolve final color
        'if (max_val2.r==0.) {alpha=0.;} else {alpha=1.;}',
        'gl_FragColor = vec4(max_val2,alpha); ',
        '}',


       
    ].join('\n')
};

