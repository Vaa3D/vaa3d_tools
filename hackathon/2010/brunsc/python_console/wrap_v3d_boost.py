#!/usr/bin/python

from pyplusplus import module_builder
from pyplusplus import function_transformers as FT
from pyplusplus.module_builder import call_policies
from pyplusplus.module_builder.call_policies import *
from pygccxml.declarations.matchers import access_type_matcher_t
from pygccxml import declarations
import commands
import os
from doxygen_doc_extractor import doxygen_doc_extractor
from optparse import OptionParser
import sys
import re

class V3DWrapper:
    def __init__(self):
        "Container for pyplusplus module builder for wrapping V3D"
        # Look for "--win64" command line argument
        parser = OptionParser()
        parser.add_option("--win64", action="store_true", dest="win64", default=False, help="Whether we are building Windows 64-bit")
        (options, args) = parser.parse_args()
        includes = []
        homedir = os.path.expanduser('~')
        for path in ['.',
                       os.path.join(homedir, 'svn/v3d_cmake/v3d_main/basic_c_fun'),
                       os.path.join(homedir, 'svn/v3d/v3d_main/basic_c_fun'),
                       os.path.join(homedir, 'Documents/svn/v3d_cmake/v3d_main/basic_c_fun'),
                       os.path.abspath('../../../v3d_main/basic_c_fun'),
                       '/usr/include/Qt',
                       '/usr/include/QtCore',
                       '/usr/include/QtGui',
                       '/usr/include/qt4',
                       '/usr/include/qt4/QtCore',
                       '/usr/include/qt4/QtGui',
                       '/Library/Frameworks/QtCore.framework/Headers',
                       '/Library/Frameworks/QtGui.framework/Headers',
                       'C:/Qt/qt-64bit-4.7.2/include/QtCore/',
                       'C:/Qt/qt-64bit-4.7.2/include/QtGui/',
                       'C:/Qt/qt-64bit-4.7.2/include/Qt/',
                       'C:/Qt/qt-64bit-4.7.2/include/',
                     ]:
            if os.path.exists(path):
                # keep command line short
                abs_path = os.path.abspath(path)
                rel_path = os.path.relpath(path)
                # print abs_path
                # print rel_path
                if len(abs_path) > len(rel_path):
                    includes.append(rel_path)
                else:
                    includes.append(abs_path)
        gccxml_executable = self.find_gccxml()
        gccxml_cflags = ' --gccxml-cxxflags "-m32"'
        # gccxml_cflags = ''
        define_symbols=[]
        if sys.platform == 'win32':
            define_symbols.append("_HAS_TR1=0")
        if options.win64:
            define_symbols.append("_WIN64")
        self.mb = module_builder.module_builder_t(
            files = ['wrappable_v3d.h',],
            gccxml_path=gccxml_executable,
            cflags=gccxml_cflags,
            include_paths=includes,
            indexing_suite_version=2,
            # ignore_gccxml_output=True,
            define_symbols=define_symbols,)
        
    def find_gccxml(self):
        g = commands.getstatusoutput("which gccxml")[1]
        if (len(g) > 0) and (os.path.exists(g)):
            return g
        for g in ['/usr/local/bin/gccxml', 
                      'C:/gccxml/bin/gccxml.exe']:
            if os.path.exists(g):
                return g
        raise Exception("Could not find gccxml program")
        
    def wrap(self):
        self.mb.add_registration_code("""
        boost::python::scope().attr("__doc__") = 
            "Python module for interacting with the V3D visualization program."
            "\\n\\n"
            "The v3d python module enables control of the "
            "V3D biomedical volume visualization program "
            "from the python programming language.  The v3d "
            "module exposes the V3D plug-in API; thus most "
            "things that can be done from a V3D plug-in can also be "
            "done interactively from the V3D python console."
            ;
        """)
        self.wrap_QList()
        self.wrap_QPolygon()
        self.wrap_QBool()
        self.wrap_QHash()
        self.mb.enum('ImagePixelType').include()
        self.mb.enum('TimePackType').include()
        self.mb.class_('V3D_GlobalSetting').include()
        self.mb.class_('NeuronTree').include()
        self.mb.class_('Image4DSimple').include()
        self.mb.class_('View3DControl').include()
        self.mb.class_('NeuronSWC').include()
        self.mb.enum('PxLocationMarkerShape').include()
        self.mb.enum('PxLocationUsefulness').include()
        self.wrap_color_vectors()
        self.wrap_Image4DSimple()
        self.wrap_TriviewControl()
        self.wrap_ImageWindow()
        self.wrap_LocationSimple()
        self.wrap_V3DPluginArgItem()
        self.wrap_callPluginFunc()
        self.wrap_c_array_struct()
        self.wrap_v3d_qt_environment()
        self.wrap_QString()
        self.wrap_Vec3()
        self.wrap_Rotation()
        self.wrap_Quaternion()
        # and finally
        self.mb.member_operators('operator*').exclude()
        self.mb.member_operators('operator->').exclude()
        self.mb.member_operators('operator++').exclude()
        self.mb.member_operators('operator--').exclude()
        self.mb.member_operators('operator>>').exclude()
        self.mb.member_operators('operator<<').exclude()
        self.mb.variables('').exclude() # avoid anonymous variable warnings
        self.mb.free_functions('qHash').exclude()

    def wrap_Quaternion(self):
        simtk = self.mb.namespace('SimTK')
        cls = simtk.class_('Quaternion_<double>')
        cls.alias = 'Quaternion'
        cls.include()
        t = cls.demangled
        cls.include_files.append("vec3_container_traits.hpp")
        cls.add_registration_code("""
            def(bp::indexing::container_suite<
                    %s,
                    bp::indexing::all_methods,
                    list_algorithms< vec_container_traits< %s > > >())
            """ % (t, t) )
        cls.member_function('normalizeThis').call_policies = \
                call_policies.return_internal_reference()        
        
    def wrap_Vec3(self):
        simtk = self.mb.namespace('SimTK')
        vec3 = simtk.class_('Vec<3, double, 1>')
        vec3.alias = 'Vec3'
        vec3.include()
        vec3.member_functions().exclude()
        vec3.member_operators().exclude()
        vec3.constructors().exclude()
        # Automatically convert python sequences to SimTK::Vec3
        self.mb.add_declaration_code('#include "convert_simtk_vec3.hpp"', tail=False)
        self.mb.add_registration_code("register_simtk_vec3_conversion();", tail=False)
        # use indexing suite to allow slicing etc.
        t = vec3.demangled
        vec3.include_files.append("vec3_container_traits.hpp")
        vec3.add_registration_code("""
            def(bp::indexing::container_suite<
                    %s,
                    bp::indexing::all_methods,
                    list_algorithms< vec_container_traits< %s > > >())
            """ % (t, t) )
        
    def wrap_Rotation(self):
        simtk = self.mb.namespace('SimTK')
        rot = simtk.class_('Rotation_<double>')
        rot.alias = "Rotation"
        rot.include()
        rot.member_functions().exclude()
        rot.constructors().exclude()
        rot.member_operators().exclude()
        # Expose only methods needed for movie maker for now
        fn = rot.member_function('setRotationFromThreeAnglesThreeAxes')
        fn.include()
        fn.call_policies = call_policies.return_internal_reference()
        simtk.enum('BodyOrSpaceType').include()
        simtk.class_('CoordinateAxis').include()
        simtk.class_('CoordinateAxis').constructors(arg_types=[None]).exclude()
        simtk.class_('CoordinateAxis').member_function('crossProduct').exclude()
        simtk.variable('XAxis').include()
        simtk.variable('YAxis').include()
        simtk.variable('ZAxis').include()
        fn = rot.member_function('convertThreeAxesRotationToThreeAngles')
        fn.include()
        fn = rot.member_function('setRotationFromQuaternion')
        fn.include()
        fn.call_policies = call_policies.return_internal_reference()
        
    def wrap_callPluginFunc(self):
        fn = self.mb.free_function("callPluginFunc")
        # fn.add_transformation(FT.output('output')) # No, we need to pass that object
        
    def wrap_V3DPluginArgItem(self):
        cls = self.mb.class_('V3DPluginArgItem')
        cls.include()
        cls.variable('p').exclude()
        cls.variable('type').exclude() # QString is converted, not exposed
        getter = self.mb.free_function("get_argitem_type")
        setter = self.mb.free_function("set_argitem_type")
        cls.add_property( 'type', getter, setter )
        getter.exclude()
        setter.exclude()
        # Harder problem than type string is the pointer to the argument object
        cls.add_declaration_code("""
            void set_pluginargitem_pointer(
                    V3DPluginArgItem& arg, 
                    bp::object& bpo) 
            {
                arg.p = &bpo;
            }
            bp::object* get_pluginargitem_pointer(
                    V3DPluginArgItem& arg) 
            {
                return static_cast<bp::object*>(arg.p);
            }
        """)
        cls.add_registration_code("""
            add_property("p",
            bp::make_function(&get_pluginargitem_pointer,
                bp::return_internal_reference<>()),
            bp::make_function(&set_pluginargitem_pointer,
                bp::with_custodian_and_ward<1,2>() ))
        """)
    
    def wrap_v3d_qt_environment(self):
        # Too bad.  this does not seem to work with pyside.
        fn = self.mb.namespace('v3d').free_function('get_qt_gui_parent')
        fn.include()
        fn.call_policies = return_value_policy(reference_existing_object)
        # Don't warn me about QWidget.  User must use PySide or PyQt4 to get QWidget.
        cls = self.mb.class_('QWidget')
        cls.include()
        cls.variables().exclude()
        cls.constructors().exclude()
        cls.member_functions().exclude()
        cls.member_operators().exclude()
        # cls.already_exposed = True
                
    def wrap_c_array_struct(self):
        self.mb.add_declaration_code('#include "convert_c_array_struct.h"', tail=False)
        for cls_name in ['c_array<unsigned char, 3>',
                         'c_array<int, 3>',
                         'c_array<unsigned char, 4>',
                         'c_array<float, 3>',
                         'c_array<short, 3>',
                         'c_array<double, 3>',
                         'c_array< c_array<double, 3>, 3 >',
                         ]:
            cls = self.mb.class_(cls_name)
            self.mb.add_registration_code(
                'register_c_array_struct_conversion< %s >();' % cls.demangled,
                tail = False)
            # cls.already_exposed = True
            cls.include()
            cls.member_functions('begin').exclude();
            cls.member_functions('end').exclude();
            cls.casting_operators().exclude(); # no casting operators, to avoid warnings
            t = cls.demangled
            cls.include_files.append("convert_c_array_struct.h")
            cls.add_registration_code("""
                def(bp::indexing::container_suite<
                    %s, bp::indexing::all_methods,
                    list_algorithms<c_array_struct_container_traits< %s > > >())
                """ % (t,t) )

    
    def wrap_QHash(self):
        self.mb.add_declaration_code('#include "convert_qhash.h"', tail=False)
        cls = self.mb.class_('QHash<int,int>')
        self.mb.add_registration_code(
                'register_qhash_conversion< %s >();' % cls.demangled, 
                tail=False)
        cls.already_exposed = True
        
    def wrap_QPolygon(self):
        cls = self.mb.class_('QPoint')
        cls.include()
        for fn_name in ['rx', 'ry']:
            cls.member_function(fn_name).call_policies = \
                    return_value_policy(copy_non_const_reference)
        cls = self.mb.class_('QSize')
        cls.include()
        for fn_name in ['rwidth', 'rheight']:
            cls.member_function(fn_name).call_policies = \
                    return_value_policy(copy_non_const_reference)
        self.mb.enum('AspectRatioMode').include()
        self.mb.enum('FillRule').include()
        cls = self.mb.class_('QRect')
        cls.include()
        cls = self.mb.class_('QPolygon')
        cls.include()
        cls.member_functions('setPoints').exclude() # variable arg list
        cls.member_functions('putPoints').exclude() # variable arg list
        cls.member_function('putPoints', arg_types=[None, None, None, None]).include()
        for op in cls.casting_operators():
            if op.name.find("QVariant") >= 0:
                op.exclude()
        
    def wrap_color_vectors(self):
        self.mb.class_('RGB8').include()
        self.mb.class_('RGB16i').include()
        self.mb.class_('RGB32i').include()
        self.mb.class_('RGB32f').include()
        self.mb.class_('RGBA8').include()
        self.mb.class_('XYZ').include()
        # Avoid warnings about casting operators
        for op in self.mb.class_('XYZ').casting_operators():
            if op.name.find("RGB") >= 0:
                op.exclude()
        
    def wrap_QBool(self):
        # Warning - this could interfere with use of PySide or PyQt
        cls = self.mb.class_('QBool')
        # cls.include_files.append("convert_qstring.h")
        self.mb.add_declaration_code('#include "convert_qbool.h"', tail=False)
        self.mb.add_registration_code('register_qbool_conversion();', tail=False)
        cls.already_exposed = True;

    def wrap_QList(self):
        self.mb.add_declaration_code('#include "convert_qlist.h"', tail=False)
        for cls_name in ['QVector<QPoint>',
                         'QList<LocationSimple>', 
                         'QList<NeuronSWC>', 
                         'QList<QPolygon>', 
                         'QList<V3DPluginArgItem>'
                         ]:
            cls = self.mb.class_(cls_name)
            self.mb.add_registration_code(
                  'register_qlist_conversion<%s >();' % cls.demangled, 
                  tail=False)
            cls.already_exposed = True
        # RuntimeError: extension class wrapper for base class QVector<QPoint> has not been created yet
        cls = self.mb.class_('QVector<QPoint>')
        cls.include()
        cls.no_init = True;
        cls.noncopyable = True;
        cls.member_functions().exclude()
        cls.member_operators().exclude()
        cls.variables().exclude()
        cls.already_exposed = False
        
    def wrap_one_QList(self, cls):
        cls.include()
        cls.variables().exclude()
        # Avoid constructor that takes Node* argument
        for ctor in cls.constructors(arg_types=[None]):
            arg_t = ctor.argument_types[0]
            if (declarations.is_pointer(arg_t)):
                ctor.exclude()
        for fn_name in ['detach_helper_grow', 
                        'node_construct', 
                        'node_destruct',
                        'node_copy',
                        'fromVector',
                        'toVector',
                        'toSet',
                        'fromSet']:
            cls.member_functions(fn_name).exclude()
        for fn_name in ['back', 'first', 'front', 'last']:
            cls.member_functions(fn_name).call_policies = \
                call_policies.return_internal_reference()
        # TODO - add python sequence operators
        cls.include_files.append("qlist_py_indexing.h")
        cls.add_registration_code("""
            def(bp::indexing::container_suite<
                    %s, 
                    bp::indexing::all_methods, 
                    list_algorithms<qlist_container_traits<%s > > >())
            """ % (cls.demangled, cls.demangled) )

        
    def wrap_QString(self):
        # Warning - this could interfere with use of PySide or PyQt
        cls = self.mb.class_('QString')
        # cls.include_files.append("convert_qstring.h")
        self.mb.add_declaration_code('#include "convert_qstring.h"', tail=False)
        self.mb.add_registration_code('register_qstring_conversion();', tail=False)
        cls.already_exposed = True;
        # cls.include()
        # cls.member_functions().exclude()
        # cls.constructors().exclude()
        # cls.member_operators().exclude()
        # cls.variables().exclude()
        
    def wrap_LocationSimple(self):
        cls = self.mb.class_('LocationSimple')
        cls.include()
        # TODO - use only the float version of getCoord, 
        # ...and push arguments to return value.
        for fn in cls.member_functions('getCoord'):
            arg_t = fn.argument_types[0]
            if (arg_t.decl_string == "int &"):
                # fn.exclude() # don't want the int one
                fn_alias = 'getCoordInt'
            else:
                # want the float one
                fn_alias = 'getCoord'
            fn.add_transformation( 
                                  FT.output('xx'), 
                                  FT.output('yy'), 
                                  FT.output('zz'),
                                  alias = fn_alias )
        
    def wrap_Image4DSimple(self):
        cls = self.mb.class_("Image4DSimple")
        cls.include()
        cls.member_functions( access_type_matcher_t( 'protected' ), 
                              allow_empty=True ).exclude()
        cls.variables( access_type_matcher_t( 'protected' ), 
                              allow_empty=True ).exclude()
        cls.member_function('getRawData').exclude() # raw pointer not useful in python
        cls.member_function('getRawDataAtChannel').exclude()
        # link errors
        cls.member_functions('loadImage').exclude()        
        cls.member_function('saveImage').exclude()        
        cls.member_function('createImage').exclude()        
        cls.member_function('createBlankImage').exclude()        
    
    def wrap_ImageWindow(self):
        self.mb.class_('ImageWindowReceiver').exclude()
        self.mb.class_('ImageWindowDispatcher').exclude()
        cls = self.mb.class_('ImageWindow')
        cls.include()
        cls.variables('handle').exclude()
        # get/set image
        fn1 = cls.member_function("getImage")
        fn1.call_policies = call_policies.return_internal_reference()
        fn2 = cls.member_function("setImage")
        fn2.call_policies = call_policies.with_custodian_and_ward(1,2)
        cls.add_property( 'image'
                     , cls.member_function( 'getImage' )
                     , cls.member_function( 'setImage' ) )
        # get/set name
        cls.add_property( 'name'
                     , cls.member_function( 'getName' )
                     , cls.member_function( 'setName' ) )
        # 3D window controls
        fn = cls.member_function("getView3DControl")
        fn.call_policies = call_policies.return_internal_reference()
        cls.add_property('view3DControl', fn)
        fn = cls.member_function("getLocalView3DControl")
        fn.call_policies = call_policies.return_internal_reference()
        cls.add_property('localView3DControl', fn)
        fn = cls.member_function("getTriviewControl")
        fn.call_policies = call_policies.return_internal_reference()
        cls.add_property('triViewControl', fn)
        # exclude constructor that takes void* argument
        for ctor in cls.constructors(arg_types = [None]):
            arg_t = ctor.argument_types[0]
            if (declarations.is_pointer(arg_t)):
                ctor.exclude()
        
    def wrap_TriviewControl(self):
        tvc = self.mb.class_('TriviewControl')
        tvc.include()
        # getFocusLocation passes int refs as args for return.
        gfl = tvc.member_function('getFocusLocation')
        gfl.add_transformation( 
                FT.output('cx'), 
                FT.output('cy'), 
                FT.output('cz') )
        fn = tvc.member_function('getTriViewColorDispType')
        fn.add_transformation(FT.output('mytype'))
        
    def write_out(self, outputDir):
        extractor = doxygen_doc_extractor()
        self.mb.build_code_creator(module_name='v3d', doc_extractor=extractor)
        self.mb.split_module(outputDir)
        # Record success for makefile dependencies: "touch" in python is long...
        open(os.path.join(os.path.abspath('.'), 'generated_code', 'generate_v3d.stamp'), "w").close()


if __name__ == "__main__":
    v3dWrapper = V3DWrapper()
    v3dWrapper.wrap()
    v3dWrapper.write_out('generated_code')
