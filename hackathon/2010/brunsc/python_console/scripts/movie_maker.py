import time
import os
import math
import v3d
from xml.dom.minidom import getDOMImplementation
import xml.dom.minidom

def _round_to_int(x):
    return int(math.floor(x + 0.5))

class Interpolator:
    def get_interpolated_value(self, param, param_value_list, index_hint = None, wrap = None):
        """
        param_value_list is an ordered list of (parameter, value) pairs
        in which the parameter parameter is strictly increasing.
        Wrap is the cycle value for cyclic systems.  For example, wrap = 360 for
        angles in degrees.
        """
        raise NotImplementedError("Use a concrete derived class of Interpolator")
    
    def check_values(self, param, param_value_list):
        # Make sure our lookup table is up to the challenge
        if len(param_value_list) < 1:
            raise LookupError("No key values to interpolate from")
        if param < param_value_list[0][0]:
            print "param = %f" % param
            print "first param = %f" % param_value_list[0][0]
            raise ValueError("interpolator parameter value too small")
        if param > param_value_list[-1][0]:
            print param, param_value_list[-1][0]
            raise ValueError
        
    def get_lower_bound_key(self, param, param_value_list, index_hint):
        # Search for bounding keys
        if index_hint != None:
            below_index = index_hint
        else:
            # TODO - binary search
            raise NotImplementedError()
        while param_value_list[below_index][0] < param:
            below_index += 1
        while param_value_list[below_index][0] > param:
            below_index -= 1
        return below_index

# TODO - create spline interpolator
# and quaternion interpolator for rotations
class LinearInterpolator(Interpolator):
    """
    Simple linear interpolator, so I can get things working quickly.
    Later we will use spline interpolation.
    """
    def get_interpolated_value(self, param, param_value_list, index_hint = None, wrap = None):
        self.check_values(param, param_value_list)
        below_index = self.get_lower_bound_key(param, param_value_list, index_hint)
        p1 = param_value_list[below_index][0]
        if param == p1:
            return param_value_list[below_index][1] # exact key frame
        p2 = param_value_list[below_index + 1][0]
        alpha = (param - p1) / (p2 - p1)
        val1 = param_value_list[below_index][1]
        val2 = param_value_list[below_index + 1][1]
        dv = val2 - val1
        if wrap != None:
            while dv > wrap / 2.0:
                dv -= wrap
            while dv < -wrap / 2.0:
                dv += wrap
        val = val1 + alpha * dv
        # print param, p1, p2, val1, val2, alpha
        return val


class SplineInterpolator(Interpolator):
    "Interpolate using Catmull Rom splines"
    def get_interpolated_value(self, param, param_value_list, index_hint = None, wrap = None):
        # There are three ways to handle boundary conditions:
        # 1) Linear - start and end at full speed
        # 2) Hold - Accelerate from zero at start, decelerate at end (covers swing case too)
        # 3) Loop - First point is treated as adjacent to final point (perhaps first and last must be same)
        self.check_values(param, param_value_list)
        below_index = self.get_lower_bound_key(param, param_value_list, index_hint)
        
        t1 = param_value_list[below_index][0]
        t2 = param_value_list[below_index + 1][0]
        v1 = param_value_list[below_index + 0][1]
        v2 = param_value_list[below_index + 1][1]
        if param == t1:
            return v1 # exact key frame
        if below_index > 0: # general case
            t0 = param_value_list[below_index - 1][0]
            v0 = param_value_list[below_index - 1][1]
            # Scale to simulate uniform interval
            v0 = v1 + (v0 - v1) * (t2 - t1) / (t1 - t0)
        else: # lower boundary condition
            # just do Hold for now
            t0 = t1 + t1 - t2
            v0 = v2
        if below_index < (len(param_value_list) - 2):
            t3 = param_value_list[below_index + 2][0]
            v3 = param_value_list[below_index + 1][1]
            # Scale to simulate uniform interval
            v3 = v2 + (v3 - v2) * (t2 - t1) / (t3 - t2)
        else:
            # Just do hold for now
            # TODO - support loop, linear
            t3 = t2 + t2 - t1
            v3 = v2
        # Wrap systems like angles
        vals = [v0, v1, v2, v3]
        for ix in [1,2,3]:
            dv = vals[ix] - vals[ix - 1]
            if wrap != None:
                while dv > wrap / 2.0:
                    dv -= wrap
                while dv < -wrap / 2.0:
                    dv += wrap
            vals[ix] = vals[ix - 1] + dv
        alpha = (param - t1) / (t2 - t1)
        val = self.spline_4points( alpha, vals )
        return val
    
    def spline_4points( self, t, points ):
        """ Catmull-Rom
            (Ps can be numpy vectors or arrays too: colors, curves ...)
            Assumes that the data points are on a uniform interval, so
            scale them beforehand if this is not the case.
        """
            # wikipedia Catmull-Rom -> Cubic_Hermite_spline
            # 0 -> p0,  1 -> p1,  1/2 -> (- p_1 + 9 p0 + 9 p1 - p2) / 16
        assert 0 <= t <= 1
        assert len(points) == 4
        return (
              t*((2.0-t)*t - 1.0)   * points[0]
            + (t*t*(3.0*t - 5.0) + 2.0) * points[1]
            + t*((4.0 - 3.0*t)*t + 1.0) * points[2]
            + (t-1.0)*t*t         * points[3] ) / 2.0

class QuaternionInterpolator(Interpolator):
    def get_interpolated_value(self, param, param_value_list, index_hint = None, wrap = None, linear = False):
        # There are three ways to handle boundary conditions:
        # 1) Linear - start and end at full speed
        # 2) Hold - Accelerate from zero at start, decelerate at end (covers swing case too)
        # 3) Loop - First point is treated as adjacent to final point (perhaps first and last must be same)
        self.check_values(param, param_value_list)
        below_index = self.get_lower_bound_key(param, param_value_list, index_hint)
        
        t1 = param_value_list[below_index][0]
        t2 = param_value_list[below_index + 1][0]
        alpha = (param - t1) / (t2 - t1)
        v1 = param_value_list[below_index + 0][1]
        v2 = param_value_list[below_index + 1][1]
        if linear: # linear only
            return self.slerp(v1, v2, alpha)
        # Catmull Rom interpolation
        if param == t1:
            return v1 # exact key frame
        if below_index > 0: # general case
            t0 = param_value_list[below_index - 1][0]
            v0 = param_value_list[below_index - 1][1]
            # Ignore non-uniformity of interval... Sorry.
        else: # lower boundary condition
            # just do Hold-ish for now
            t0 = t1 + t1 - t2
            v0 = v1
        if below_index < (len(param_value_list) - 2): # general case
            t3 = param_value_list[below_index + 2][0]
            v3 = param_value_list[below_index + 1][1]
            # Ignore non-uniformity of interval... Sorry.
        else:
            # Just do hold-ish for now
            # TODO - support loop, linear
            t3 = t2 + t2 - t1
            v3 = v2
        return self.catmull_quat([v0, v1, v2, v3], alpha)
    
    def quat_dot(self, q1, q2):
        result = 0.0
        for ix in range(4):
            result += q1[ix] * q2[ix]
        return result
    
    def slerp(self, q1, q2, alpha, spin=0, debug=False, bFlip=None):
        """
        Spherical linear interpolation of quaternions.
        param alpha is between 0.0 and 1.0, but is allow outside that range
        Returns an interpolated quaternion.
        Adapted from appendix E of Visualizing Quaternions by AJ Hanson
        """
        if debug: debug = "  slerp debug"
        # assert 0 <= alpha <= 1.0
        # cosine theta = dot product of a and b
        cos_t = self.quat_dot(q1,q2)
        if debug: debug += " %f" % cos_t
        # if B is on opposite hemisphere from A, use -B instead
        # ...unless we have been told the bFlip value
        if None == bFlip:
            bFlip = False
            if cos_t < 0.0:
                bFlip = True
        if bFlip:
            cos_t = -cos_t
        # If B is (within precision limits) the same as A,
        # just linear interpolate between A and B.
        # Can't do spins, since we don't know what direction to spin.
        if (1.0 - abs(cos_t)) < 1e-7:
            if debug: debug += " linear"
            beta = 1.0 - alpha
        else: # normal case
            theta = math.acos(cos_t)
            phi = theta + spin * math.pi
            sin_t = math.sin(theta)
            beta = math.sin(theta - alpha*phi) / sin_t
            alpha = math.sin(alpha*phi) / sin_t
        if bFlip:
            alpha = -alpha
            if debug: debug += " bFlip"
        result = v3d.Quaternion()
        for i in range(4):
            result[i] = beta * q1[i] + alpha * q2[i]
        if debug: debug += str(result[:])
        result.normalizeThis()
        if debug: debug += str(result[:])
        if debug: debug += " %f, %f" % (beta, alpha)
        if debug: print debug
        return result

    def _bFlip(self, qA, qB):
        """
        Whether quaternions are in the same hemisphere
        """
        return (self.quat_dot(qA,qB) < 0.0)
        
        
    def catmull_quat(self, quats, t):
        """
        Interpolate orientations using Catmull-Rom splines in quaternion space.
        Adapted from appendix E of Visualizing Quaternions by AJ Hanson
        """
        # Put all control points in the same hemisphere as q01.
        # ...then hardcode slerps to never bFlip, to avoid unwanted
        # discontinuities during interpolation.
        q0 = quats[:]
        if self._bFlip(q0[0], q0[1]):
            q = v3d.Quaternion(q0[0]) # copy
            q[:] = map(lambda x: -x, q) # negate
            q0[0] = q
        if self._bFlip(q0[1], q0[2]):
            q = v3d.Quaternion(q0[2]) # copy
            q[:] = map(lambda x: -x, q) # negate
            q0[2] = q
        # ...well, actually put q03 in same hemisphere as q02
        if self._bFlip(q0[2], q0[3]):
            q = v3d.Quaternion(q0[3]) # copy
            q[:] = map(lambda x: -x, q) # negate
            q0[3] = q
        q10 = self.slerp(q0[0], q0[1], t+1.0, bFlip=False)
        q11 = self.slerp(q0[1], q0[2], t, bFlip=False)
        q12 = self.slerp(q0[2], q0[3], t-1.0, bFlip=False)
        q20 = self.slerp(q10, q11, (t+1.0)/2.0, bFlip=False)
        q21 = self.slerp(q11, q12, t/2.0, bFlip=False)
        # debug
        # print "  ", q10[:], q11[:], q12[:], q20[:], q21[:], t
        # print "  q20 = ", q20[:], t
        return self.slerp(q20, q21, t)


# TODO - include rotation and clipping
#      - And maybe eventually list of displayed objects
#        perhaps that list should be in a different object
class CameraPosition:
    def __init__(self, **kw_params):
        for param_name in kw_params:
            setattr(self, param_name, kw_params[param_name])

    def populate_quaterion(self):
        # convert to radians
        DegToRad = math.pi / 180.0
        xRot = self.xRot * DegToRad
        yRot = self.yRot * DegToRad
        zRot = self.zRot * DegToRad
        # construct rotation matrix
        R = v3d.Rotation()
        R.setRotationFromThreeAnglesThreeAxes(
                v3d.BodyRotationSequence, 
                xRot, v3d.XAxis, 
                yRot, v3d.YAxis, 
                zRot, v3d.ZAxis)
        self.quaternion = v3d.Quaternion(R)
        

class V3dMovieFrame:
    """
    Any single frame of a V3D movie.
    
    Includes both key frames and in-between frames
    """
    def __init__(self, camera_position):
        self.camera_position = camera_position


class V3dKeyFrame(V3dMovieFrame):
    def __init__(self, camera_position, interval = 0):
        V3dMovieFrame.__init__(self, camera_position)
        self.interval = interval # in seconds from previous frame
        self.interpolator = SplineInterpolator()
        self.quat_interpolator = QuaternionInterpolator()


class V3dMovie:
    def _find_image_window(self):
        try:
            self.image_window = v3d.ImageWindow.current()
        except:
            self.image_window = None
            
    def _find_view_control(self):
        if self.image_window == None:
            self._find_image_window()
        if self.image_window == None:
            return
        self.view_control = self.image_window.getView3DControl()
        # Perhaps the user is using a local 3D window instead
        if self.view_control == None:
            self.view_control = self.image_window.getLocalView3DControl()
        
    def __init__(self, seconds_per_frame=1.0/24.0):
        import v3d
        self.seconds_per_frame = seconds_per_frame # seconds per frame, default 24 fps
        self.key_frames = []
        self._find_image_window()
        self._find_view_control()
        # interpolation_param_names are names of View3DControl getter/getter methods
        self.view_control_param_names = {
                                        'xShift' : 'setXShift', 
                                        'yShift' : 'setYShift', 
                                        'zShift' : 'setZShift',
                                        'xRot' : 'setXRotation', 
                                        'yRot' : 'setYRotation', 
                                        'zRot' : 'setZRotation',
                                        'zoom' : 'setZoom',
                                        'xCut0' : 'setXCut0', 
                                        'xCut1' : 'setXCut1',
                                        'yCut0' : 'setYCut0', 
                                        'yCut1' : 'setYCut1',
                                        'zCut0' : 'setZCut0', 
                                        'zCut1' : 'setZCut1',
                                        'xCS' : 'setXCS',
                                        'yCS' : 'setYCS',
                                        'zCS' : 'setZCS',
                                        'channelR' : 'setChannelR',
                                        'channelG' : 'setChannelG',
                                        'channelB' : 'setChannelB',
                                        'frontCut' : 'setFrontCut',
                                        'volumeTimePoint' : 'setVolumeTimePoint',
                                        # need special logic to set renderMode
                                        # ; these getters do not exist
                                        'renderMode_Cs3d' : 'setRenderMode_Cs3d',
                                        'renderMode_Mip' : 'setRenderMode_Mip',
                                        'renderMode_Alpha' : 'setRenderMode_Alpha',
                                        }

    def _setup_interpolation_lists(self):
        # Create dictionary for lists of parameters
        self.interpolation_list = {}
        elapsed_time = 0.0
        # Create empty list for each parameter
        for param_name in self.view_control_param_names:
            self.interpolation_list[param_name] = []
        self.interpolation_list['quaternion'] = []
        # Populate parameter lists with one entry per key-frame
        # Each entry in a parameter lists holds a [time, value] pair
        frame_count = 0
        for key_frame in self.key_frames:
            frame_count += 1
            # Don't advance time before first frame
            if frame_count > 1:
                elapsed_time += key_frame.interval
            for param_name in self.view_control_param_names:
                self.interpolation_list[param_name].append([elapsed_time, 
                            getattr(key_frame.camera_position, param_name)],)
            # Handle quaternion rotation explicitly
            self.interpolation_list['quaternion'].append(
                    [elapsed_time, key_frame.camera_position.quaternion])
        
    def play(self):
        """
        Play back movie frames at a rate no faster than real-time.
        """
        # Use a generator, to provide an opportunity to update GUI between frames
        for elapsed_time in self.generate_play_frames():
            pass
            
    def generate_play_frames(self):
        """
        Play back movie frames at a rate no faster than real-time.
        One frame at a time, as a python generator.
        Returns elapsed time.
        """
        #
        movie_start_clocktime = time.clock()
        frame_start_clocktime = movie_start_clocktime
        movie_elapsed_time = 0.0
        frame_number = 0
        for frame in self.generate_frame_views():
            # Are we playing too fast?
            now = time.clock()
            elapsed_frame_time = now - frame_start_clocktime
            frame_start_clocktime = now
            real_time_deficit = self.seconds_per_frame - elapsed_frame_time
            if real_time_deficit > 0:
                time.sleep(real_time_deficit)
            frame_start_clocktime = now
            frame_number += 1
            yield movie_elapsed_time
            movie_elapsed_time += self.seconds_per_frame

    def _frame_name(self, dir, root, num):
        fname = "%s_frame_%05d.BMP" % (root, num)
        return os.path.join(dir, fname)

    def write_frames(self, directory, file_root=None):
        for frame_number in self.generate_write_frames(directory, file_root):
            pass
        
    def generate_write_frames(self, directory, file_root=None):        
        """
        Writes movie to disk, with one BMP file per movie frame.
        
        Parameters:
          directory -- directory path where frames will be saved
        
          file_root -- base name for frame files.  For example, a file_root
            of "foo" would result in frame files like "foo_frame_00001.BMP",
            "foo_frame_00002.BMP", etc.  If file_root is not specified,
            the file name is based on the name of the V3D image.
            
        Unlike V3dMovie.play(), V3dMovie.write_frames() might play back
        the movie faster than real time, depending on rendering and I/O
        speed.
        """
        # Make sure the file_root name is OK
        if file_root == None:
            file_root = self.image_window.name
            file_root = os.path.split(file_root)[-1] # remove directory name
            file_root = os.path.splitext(file_root)[0] # remove file extension
        if len(file_root) < 1:
            file_root = "v3d"
        # Modify file name if such frames already exist
        file_root0 = file_root
        file_root_ver = 2
        while os.path.exists(self._frame_name(directory, file_root, 1)):
            file_root = "%s_%d" % (file_root0, file_root_ver)
            file_root_ver += 1
        # Actually write the frames
        frame_number = 0
        for frame in self.generate_frame_views():
            frame_number += 1
            file_name = self._frame_name(directory, file_root, frame_number)
            # print "Writing frame image file %s" % file_name
            if self.image_window:
                self.image_window.screenShot3DWindow(file_name[0:-4]) # strip off ".BMP"
            yield frame_number
                
    def interpolate_frame(self, elapsed_time, frame_index_hint, interpolator, quat_interpolator):
        """
        Returns an in-between frame.
        
        elapsed_time -- the amount of time between the start of the movie and the desired frame
        
        frame_index_hint is the index of a nearby key frame
        
        interpolator -- Interpolator object that can compute the frame parameters
        """
        camera_position = CameraPosition()
        for param_name in self.view_control_param_names:
            wrap = None
            # Rotation angles need to be interpolated in a Z system
            if 'Rot' in param_name:
                wrap = 360
            interp_val = interpolator.get_interpolated_value(
                elapsed_time,
                self.interpolation_list[param_name],
                frame_index_hint,
                wrap = wrap)
            setattr(camera_position, param_name, interp_val)
        camera_position.quaternion = quat_interpolator.get_interpolated_value(
                    elapsed_time,
                    self.interpolation_list['quaternion'],
                    frame_index_hint)
        return V3dMovieFrame(camera_position)
        
    def generate_final_frame_view(self):
        frame = self.key_frames[-1]
        self.set_current_v3d_camera(frame.camera_position)
        self.image_window.update()
        
    def generate_frame_views(self):
        """
        Unlike generate_frame_info(), generate_frame_views() actually
        adjusts the current view in the V3D 3D viewer.
        """
        # self.image_window.open3DWindow()
        # Turn off cut plane locks
        if None == self.view_control: 
            return
        self.view_control.setXCutLock(False)
        self.view_control.setYCutLock(False)
        self.view_control.setZCutLock(False)
        for frame in self.generate_frame_info():
            self.set_current_v3d_camera(frame.camera_position)
            self.image_window.update() # The obviates need to print to get window update
            yield frame
            
    def generate_frame_info(self):
        "Generator to produce each frame object of the movie, one by one"
        self._setup_interpolation_lists()
        total_time = 0.0
        frame_index = 0
        for key_frame in self.key_frames:
            # also avoid too many frames from roundoff error
            frame_time = self.seconds_per_frame * 1.01
            # First emit in-between frames
            # print key_frame.interval
            # But not before first frame
            if frame_index > 0:
                while frame_time < key_frame.interval:
                    # TODO interpolate
                    yield self.interpolate_frame(
                                total_time, frame_index, 
                                key_frame.interpolator,
                                key_frame.quat_interpolator)
                    # yield V3dMovieFrame() # TODO interpolate
                    frame_time += self.seconds_per_frame
                    total_time += self.seconds_per_frame
            # Then emit the key frame
            yield key_frame
            total_time += self.seconds_per_frame
            frame_index += 1
            
    def set_current_v3d_camera(self, camera_position):
        if not self.view_control:
            self._find_view_control()
        if not self.view_control:
            raise ValueError("No V3D window is attached")
        # print "Setting view control..."
        for getter_name in self.view_control_param_names:
            setter_name = self.view_control_param_names[getter_name]
            if 'Rot' in getter_name:
                continue # rotation is handled specially, below
            val = getattr(camera_position, getter_name)
            if 'Cut' in getter_name:
                val = _round_to_int(val) # Cut methods take integer arguments
            # if 'yCut1' == getter_name: # debugging
            #     print getter_name, val
            if 'CS' in getter_name:
                val = _round_to_int(val)
            if 'olumeTimePoint' in getter_name:
                val = _round_to_int(val)
            if 'hannel' in getter_name:
                # needs to be boolean
                val = (val > 0.5)
            if 'renderMode' in getter_name:
                val = (val > 0.5) # boolean
            fn = getattr(self.view_control, setter_name)
            fn(val) # set parameter in V3D view_control
            # print "Setting parameter %s to %s" % (getter_name, val)
        # For some reason set[XYZ]Rotation() does an incremental change
        R = v3d.Rotation()
        R.setRotationFromQuaternion(camera_position.quaternion)
        angles = R.convertThreeAxesRotationToThreeAngles(
                    v3d.BodyRotationSequence,
                    v3d.XAxis,
                    v3d.YAxis,
                    v3d.ZAxis)
        RadToDeg = 180.0 / math.pi
        self.view_control.doAbsoluteRot(
                    angles[0] * RadToDeg,
                    angles[1] * RadToDeg,
                    angles[2] * RadToDeg)
        # debugging
        # q = camera_position.quaternion
        # print "Quaternion: ", q[:], "Euler angles: ", angles[:]

    def get_current_v3d_camera(self):
        if not self.view_control:
            self._find_view_control()
        if not self.view_control:
            raise ValueError("No V3D window is attached")
        # TODO absoluteRotPose() changes the view slightly.  I don't think it should...
        self.view_control.absoluteRotPose()
        camera = CameraPosition()
        for param_name in self.view_control_param_names:
            if 'renderMode' in param_name:
                continue
            val = getattr(self.view_control, param_name)()
            # if 'yCut1' == param_name: # debugging
            #     print param_name, val
            setattr(camera, param_name, val)
            # print "Parameter %s = %s" % (param_name, val)
        # Handle render mode explicitly
        # enum RenderMode {rmCrossSection=0, rmAlphaBlending, rmMaxIntensityProjection };
        # virtual void setRenderMode_Mip(bool b);
        # virtual void setRenderMode_Alpha(bool b);
        # virtual void setRenderMode_Cs3d(bool b);
        rm = self.view_control.renderMode()
        camera.renderMode_Cs3d = (rm == 0)
        camera.renderMode_Alpha = (rm == 1)
        camera.renderMode_Mip = (rm == 2) # boolean        
        # Rotation we handle explicitly
        camera.populate_quaterion()
        return camera
        
    def append_current_view(self, interval=2.0):
        camera = self.get_current_v3d_camera()
        if len(self.key_frames) == 0:
            interval = 0.0
        self.key_frames.append(V3dKeyFrame(camera_position = camera, 
                                           interval = interval))
        
    def save_parameter_file(self, file_object):
        """
        Writes an xml file containing the movie parameters.
        file_object parameter must have a write() method.
        """
        paramDocument = getDOMImplementation().createDocument(None, "v3d_movie", None)
        root_element = paramDocument.documentElement
        root_element.setAttribute("frames_per_second", "%.6f" % (1.0/self.seconds_per_frame) )
        for key_frame in self.key_frames:
            frame_element = paramDocument.createElement("key_frame")
            frame_element.setAttribute("interval", "%.2f" % key_frame.interval)
            camera = key_frame.camera_position
            for getter_name in self.view_control_param_names:
                frame_element.setAttribute(getter_name, str(getattr(camera, getter_name)))
            root_element.appendChild(frame_element)
        paramDocument.writexml(file_object, addindent="  ", newl="\n")
        
    def load_parameter_file(self, file_object):
        """
        Reads an xml file containing the movie parameters.
        """
        paramDocument = xml.dom.minidom.parse(file_object)
        root_element = paramDocument.documentElement
        self.seconds_per_frame = 1.0 / float(root_element.getAttribute("frames_per_second"))
        self.key_frames = []
        for frame_element in root_element.getElementsByTagName("key_frame"):
            interval = float(frame_element.getAttribute("interval"))
            camera = CameraPosition()
            for getter_name in self.view_control_param_names:
                val = frame_element.getAttribute(getter_name)
                if "False" == val: 
                    val = False
                elif "True" == val: 
                    val = True
                else: 
                    val = int(val)
                setattr(camera, getter_name, val)
                # print getter_name, val
            camera.populate_quaterion()
            self.key_frames.append(V3dKeyFrame(camera_position = camera, 
                                   interval = interval))


# Standard python technique for optionally running this file as
# a program instead of as a library.
# print "__name__ = %s" % __name__
if __name__ == '__main__':
    pass
