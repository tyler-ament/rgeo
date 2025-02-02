/*
  Line string methods for GEOS wrapper
*/


#include "preface.h"

#ifdef RGEO_GEOS_SUPPORTED

#include <string.h>
#include <ruby.h>
#include <geos_c.h>

#include "globals.h"

#include "factory.h"
#include "geometry.h"
#include "point.h"
#include "line_string.h"

#include "coordinates.h"

RGEO_BEGIN_C


static VALUE method_line_string_geometry_type(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  if (self_data->geom) {
    result = rgeo_feature_line_string_module;
  }
  return result;
}


static VALUE method_linear_ring_geometry_type(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  if (self_data->geom) {
    result = rgeo_feature_linear_ring_module;
  }
  return result;
}


static VALUE method_line_geometry_type(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  if (self_data->geom) {
    result = rgeo_feature_line_module;
  }
  return result;
}


static VALUE method_line_string_length(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  double len;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;
  if (self_geom) {
    if (GEOSLength_r(self_data->geos_context, self_geom, &len)) {
      result = rb_float_new(len);
    }
  }
  return result;
}


static VALUE method_line_string_num_points(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;
  if (self_geom) {
    result = INT2NUM(GEOSGetNumCoordinates_r(self_data->geos_context, self_geom));
  }
  return result;
}


static VALUE method_line_string_coordinates(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  const GEOSCoordSequence* coord_sequence;
  int zCoordinate;

  GEOSContextHandle_t context;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;

  if (self_geom) {
    zCoordinate = RGEO_FACTORY_DATA_PTR(self_data->factory)->flags & RGEO_FACTORYFLAGS_SUPPORTS_Z_OR_M;
    context = self_data->geos_context;
    coord_sequence = GEOSGeom_getCoordSeq_r(context, self_geom);
    if(coord_sequence) {
      result = extract_points_from_coordinate_sequence(context, coord_sequence, zCoordinate);
    }
  }
  return result;
}



static VALUE get_point_from_coordseq(VALUE self, const GEOSCoordSequence* coord_seq, unsigned int i, char has_z)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  GEOSContextHandle_t self_context;
  double x, y, z;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_context = self_data->geos_context;
  if (GEOSCoordSeq_getX_r(self_context, coord_seq, i, &x)) {
    if (GEOSCoordSeq_getY_r(self_context, coord_seq, i, &y)) {
      if (has_z) {
        if (!GEOSCoordSeq_getZ_r(self_context, coord_seq, i, &z)) {
          z = 0.0;
        }
      }
      else {
        z = 0.0;
      }
      result = rgeo_create_geos_point(self_data->factory, x, y, z);
    }
  }
  return result;
}


static VALUE method_line_string_point_n(VALUE self, VALUE n)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  GEOSContextHandle_t self_context;
  const GEOSCoordSequence* coord_seq;
  char has_z;
  int si;
  unsigned int i;
  unsigned int size;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;
  if (self_geom) {
    self_context = self_data->geos_context;
    coord_seq = GEOSGeom_getCoordSeq_r(self_context, self_geom);
    if (coord_seq) {
      has_z = (char)(RGEO_FACTORY_DATA_PTR(self_data->factory)->flags & RGEO_FACTORYFLAGS_SUPPORTS_Z_OR_M);
      si = NUM2INT(n);
      if (si >= 0) {
        i = si;
        if (GEOSCoordSeq_getSize_r(self_context, coord_seq, &size)) {
          if (i < size) {
            result = get_point_from_coordseq(self, coord_seq, i, has_z);
          }
        }
      }
    }
  }
  return result;
}


static VALUE method_line_string_points(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  GEOSContextHandle_t self_context;
  const GEOSCoordSequence* coord_seq;
  char has_z;
  unsigned int size;
  unsigned int i;
  VALUE point;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;
  if (self_geom) {
    self_context = self_data->geos_context;
    coord_seq = GEOSGeom_getCoordSeq_r(self_context, self_geom);
    if (coord_seq) {
      has_z = (char)(RGEO_FACTORY_DATA_PTR(self_data->factory)->flags & RGEO_FACTORYFLAGS_SUPPORTS_Z_OR_M);
      if (GEOSCoordSeq_getSize_r(self_context, coord_seq, &size)) {
        result = rb_ary_new2(size);
        for (i=0; i<size; ++i) {
          point = get_point_from_coordseq(self, coord_seq, i, has_z);
          if (!NIL_P(point)) {
            rb_ary_store(result, i, point);
          }
        }
      }
    }
  }
  return result;
}


static VALUE method_line_string_start_point(VALUE self)
{
  return method_line_string_point_n(self, INT2NUM(0));
}


static VALUE method_line_string_end_point(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  unsigned int n;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;
  if (self_geom) {
    n = GEOSGetNumCoordinates_r(self_data->geos_context, self_geom);
    if (n > 0) {
      result = method_line_string_point_n(self, INT2NUM(n-1));
    }
  }
  return result;
}

static VALUE method_line_string_project_point(VALUE self, VALUE point)
{
  RGeo_FactoryData* factory_data;
  VALUE result = Qnil;
  VALUE factory;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  const GEOSGeometry *geos_point;

  double location;

  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  factory = self_data->factory;
  self_geom = self_data->geom;
  factory_data = RGEO_FACTORY_DATA_PTR(factory);

  if(self_geom && point) {
    geos_point = rgeo_convert_to_geos_geometry(factory, point, rgeo_geos_point_class);
    location = GEOSProject_r(self_data->geos_context, self_geom, geos_point);
    result = DBL2NUM(location);
  }
  return result;
}

static VALUE method_line_string_interpolate_point(VALUE self, VALUE loc_num)
{
  RGeo_FactoryData* factory_data;
  VALUE result = Qnil;
  VALUE factory;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  GEOSGeometry* geos_point;

  double location;

  location = NUM2DBL(loc_num);
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  factory = self_data->factory;
  factory_data = RGEO_FACTORY_DATA_PTR(factory);
  self_geom = self_data->geom;

  if(self_geom) {
    geos_point = GEOSInterpolate_r(self_data->geos_context, self_geom, location);
    result = rgeo_wrap_geos_geometry(factory, geos_point, rgeo_geos_point_class);
  }

  return result;
}

static VALUE method_line_string_is_closed(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;
  if (self_geom) {
    result = rgeo_is_geos_line_string_closed(self_data->geos_context, self_geom);
  }
  return result;
}


static VALUE method_line_string_is_ring(VALUE self)
{
  VALUE result;
  RGeo_GeometryData* self_data;
  const GEOSGeometry* self_geom;
  char val;

  result = Qnil;
  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  self_geom = self_data->geom;
  if (self_geom) {
    val = GEOSisRing_r(self_data->geos_context, self_geom);
    if (val == 0) {
      result = Qfalse;
    }
    else if (val == 1) {
      result = Qtrue;
    }
  }
  return result;
}


static VALUE method_line_string_eql(VALUE self, VALUE rhs)
{
  VALUE result;
  RGeo_GeometryData* self_data;

  result = rgeo_geos_klasses_and_factories_eql(self, rhs);
  if (RTEST(result)) {
    self_data = RGEO_GEOMETRY_DATA_PTR(self);
    result = rgeo_geos_geometries_strict_eql(self_data->geos_context, self_data->geom, RGEO_GEOMETRY_DATA_PTR(rhs)->geom);
  }
  return result;
}


static VALUE method_line_string_hash(VALUE self)
{
  st_index_t hash;
  RGeo_GeometryData* self_data;
  VALUE factory;

  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  factory = self_data->factory;
  hash = rb_hash_start(0);
  hash = rgeo_geos_objbase_hash(factory, rgeo_feature_line_string_module, hash);
  hash = rgeo_geos_coordseq_hash(self_data->geos_context, self_data->geom, hash);
  return LONG2FIX(rb_hash_end(hash));
}


static VALUE method_linear_ring_hash(VALUE self)
{
  st_index_t hash;
  RGeo_GeometryData* self_data;
  VALUE factory;

  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  factory = self_data->factory;
  hash = rb_hash_start(0);
  hash = rgeo_geos_objbase_hash(factory, rgeo_feature_linear_ring_module, hash);
  hash = rgeo_geos_coordseq_hash(self_data->geos_context, self_data->geom, hash);
  return LONG2FIX(rb_hash_end(hash));
}


static VALUE method_line_hash(VALUE self)
{
  st_index_t hash;
  RGeo_GeometryData* self_data;
  VALUE factory;

  self_data = RGEO_GEOMETRY_DATA_PTR(self);
  factory = self_data->factory;
  hash = rb_hash_start(0);
  hash = rgeo_geos_objbase_hash(factory, rgeo_feature_line_module, hash);
  hash = rgeo_geos_coordseq_hash(self_data->geos_context, self_data->geom, hash);
  return LONG2FIX(rb_hash_end(hash));
}


static GEOSCoordSequence* coord_seq_from_array(VALUE factory, VALUE array, char close)
{
  RGeo_FactoryData* factory_data;
  VALUE point_type;
  unsigned int len;
  char has_z;
  unsigned int dims;
  double* coords;
  GEOSContextHandle_t context;
  unsigned int i;
  char good;
  const GEOSGeometry* entry_geom;
  const GEOSCoordSequence* entry_cs;
  double x;
  GEOSCoordSequence* coord_seq;

  Check_Type(array, T_ARRAY);
  factory_data = RGEO_FACTORY_DATA_PTR(factory);
  point_type = rgeo_feature_point_module;
  len = (unsigned int)RARRAY_LEN(array);
  has_z = (char)(RGEO_FACTORY_DATA_PTR(factory)->flags & RGEO_FACTORYFLAGS_SUPPORTS_Z_OR_M);
  dims = has_z ? 3 : 2;
  coords = ALLOC_N(double, len == 0 ? 1 : len * dims);
  if (!coords) {
    return NULL;
  }
  context = factory_data->geos_context;
  for (i=0; i<len; ++i) {
    good = 0;
    entry_geom = rgeo_convert_to_geos_geometry(factory, rb_ary_entry(array, i), point_type);
    if (entry_geom) {
      entry_cs = GEOSGeom_getCoordSeq_r(context, entry_geom);
      if (entry_cs) {
        if (GEOSCoordSeq_getX_r(context, entry_cs, 0, &x)) {
          coords[i*dims] = x;
          if (GEOSCoordSeq_getY_r(context, entry_cs, 0, &x)) {
            coords[i*dims+1] = x;
            good = 1;
            if (has_z) {
              if (GEOSCoordSeq_getZ_r(context, entry_cs, 0, &x)) {
                coords[i*dims+2] = x;
              }
              else {
                good = 0;
              }
            }
          }
        }
      }
    }
    if (!good) {
      free(coords);
      return NULL;
    }
  }
  if (len > 0 && close) {
    if (coords[0] == coords[(len-1)*dims] && coords[1] == coords[(len-1)*dims+1]) {
      close = 0;
    }
  }
  else {
    close = 0;
  }
  coord_seq = GEOSCoordSeq_create_r(context, len + close, 3);
  if (coord_seq) {
    for (i=0; i<len; ++i) {
      GEOSCoordSeq_setX_r(context, coord_seq, i, coords[i*dims]);
      GEOSCoordSeq_setY_r(context, coord_seq, i, coords[i*dims+1]);
      GEOSCoordSeq_setZ_r(context, coord_seq, i, has_z ? coords[i*dims+2] : 0);
    }
    if (close) {
      GEOSCoordSeq_setX_r(context, coord_seq, len, coords[0]);
      GEOSCoordSeq_setY_r(context, coord_seq, len, coords[1]);
      GEOSCoordSeq_setZ_r(context, coord_seq, len, has_z ? coords[2] : 0);
    }
  }
  free(coords);
  return coord_seq;
}


static VALUE cmethod_create_line_string(VALUE module, VALUE factory, VALUE array)
{
  VALUE result;
  GEOSCoordSequence* coord_seq;
  RGeo_FactoryData* factory_data;
  GEOSGeometry* geom;

  result = Qnil;
  coord_seq = coord_seq_from_array(factory, array, 0);
  if (coord_seq) {
    factory_data = RGEO_FACTORY_DATA_PTR(factory);
    geom = GEOSGeom_createLineString_r(factory_data->geos_context, coord_seq);
    if (geom) {
      result = rgeo_wrap_geos_geometry(factory, geom, rgeo_geos_line_string_class);
    }
  }
  return result;
}


static VALUE cmethod_create_linear_ring(VALUE module, VALUE factory, VALUE array)
{
  VALUE result;
  GEOSCoordSequence* coord_seq;
  RGeo_FactoryData* factory_data;
  GEOSGeometry* geom;

  result = Qnil;
  coord_seq = coord_seq_from_array(factory, array, 1);
  if (coord_seq) {
    factory_data = RGEO_FACTORY_DATA_PTR(factory);
    geom = GEOSGeom_createLinearRing_r(factory_data->geos_context, coord_seq);
    if (geom) {
      result = rgeo_wrap_geos_geometry(factory, geom, rgeo_geos_linear_ring_class);
    }
  }
  return result;
}


static void populate_geom_into_coord_seq(GEOSContextHandle_t context, const GEOSGeometry* geom, GEOSCoordSequence* coord_seq, unsigned int i, char has_z)
{
  const GEOSCoordSequence* cs;
  double x;

  cs = GEOSGeom_getCoordSeq_r(context, geom);
  x = 0;
  if (cs) {
    GEOSCoordSeq_getX_r(context, cs, 0, &x);
  }
  GEOSCoordSeq_setX_r(context, coord_seq, i, x);
  x = 0;
  if (cs) {
    GEOSCoordSeq_getY_r(context, cs, 0, &x);
  }
  GEOSCoordSeq_setY_r(context, coord_seq, i, x);
  x = 0;
  if (has_z && cs) {
    GEOSCoordSeq_getZ_r(context, cs, 0, &x);
  }
  GEOSCoordSeq_setZ_r(context, coord_seq, i, x);
}


static VALUE cmethod_create_line(VALUE module, VALUE factory, VALUE start, VALUE end)
{
  VALUE result;
  RGeo_FactoryData* factory_data;
  char has_z;
  VALUE point_type;
  GEOSContextHandle_t context;
  const GEOSGeometry* start_geom;
  const GEOSGeometry* end_geom;
  GEOSCoordSequence* coord_seq;
  GEOSGeometry* geom;

  result = Qnil;
  factory_data = RGEO_FACTORY_DATA_PTR(factory);
  has_z = (char)(factory_data->flags & RGEO_FACTORYFLAGS_SUPPORTS_Z_OR_M);
  point_type = rgeo_feature_point_module;
  context = factory_data->geos_context;

  start_geom = rgeo_convert_to_geos_geometry(factory, start, point_type);
  if (start_geom) {
    end_geom = rgeo_convert_to_geos_geometry(factory, end, point_type);
    if (end_geom) {
      coord_seq = GEOSCoordSeq_create_r(context, 2, 3);
      if (coord_seq) {
        populate_geom_into_coord_seq(context, start_geom, coord_seq, 0, has_z);
        populate_geom_into_coord_seq(context, end_geom, coord_seq, 1, has_z);
        geom = GEOSGeom_createLineString_r(context, coord_seq);
        if (geom) {
          result = rgeo_wrap_geos_geometry(factory, geom, rgeo_geos_line_class);
        }
      }
    }
  }

  return result;
}


static VALUE impl_copy_from(VALUE klass, VALUE factory, VALUE original, char subtype)
{
  VALUE result;
  const GEOSGeometry* original_geom;
  GEOSContextHandle_t context;
  const GEOSCoordSequence* original_coord_seq;
  GEOSCoordSequence* coord_seq;
  GEOSGeometry* geom;

  result = Qnil;
  original_geom = RGEO_GEOMETRY_DATA_PTR(original)->geom;
  if (original_geom) {
    context = RGEO_FACTORY_DATA_PTR(factory)->geos_context;
    if (subtype == 1 && GEOSGetNumCoordinates_r(context, original_geom) != 2) {
      original_geom = NULL;
    }
    if (original_geom) {
      original_coord_seq = GEOSGeom_getCoordSeq_r(context, original_geom);
      if (original_coord_seq) {
        coord_seq = GEOSCoordSeq_clone_r(context, original_coord_seq);
        if (coord_seq) {
          geom = subtype == 2 ? GEOSGeom_createLinearRing_r(context, coord_seq) : GEOSGeom_createLineString_r(context, coord_seq);
          if (geom) {
            result = rgeo_wrap_geos_geometry(factory, geom, klass);
          }
        }
      }
    }
  }
  return result;
}


static VALUE cmethod_line_string_copy_from(VALUE klass, VALUE factory, VALUE original)
{
  return impl_copy_from(klass, factory, original, 0);
}


static VALUE cmethod_line_copy_from(VALUE klass, VALUE factory, VALUE original)
{
  return impl_copy_from(klass, factory, original, 1);
}


static VALUE cmethod_linear_ring_copy_from(VALUE klass, VALUE factory, VALUE original)
{
  return impl_copy_from(klass, factory, original, 2);
}


void rgeo_init_geos_line_string()
{
  VALUE geos_line_string_methods;
  VALUE geos_linear_ring_methods;
  VALUE geos_line_methods;

  // Class methods for CAPILineStringImpl
  rb_define_module_function(rgeo_geos_line_string_class, "create", cmethod_create_line_string, 2);
  rb_define_module_function(rgeo_geos_line_string_class, "_copy_from", cmethod_line_string_copy_from, 2);

  // Class methods for CAPILinearRingImpl
  rb_define_module_function(rgeo_geos_linear_ring_class, "create", cmethod_create_linear_ring, 2);
  rb_define_module_function(rgeo_geos_linear_ring_class, "_copy_from", cmethod_linear_ring_copy_from, 2);

  // Class methods for CAPILineImpl
  rb_define_module_function(rgeo_geos_line_class, "create", cmethod_create_line, 3);
  rb_define_module_function(rgeo_geos_line_class, "_copy_from", cmethod_line_copy_from, 2);

  // CAPILineStringMethods module
  geos_line_string_methods = rb_define_module_under(rgeo_geos_module, "CAPILineStringMethods");
  rb_define_method(geos_line_string_methods, "rep_equals?", method_line_string_eql, 1);
  rb_define_method(geos_line_string_methods, "eql?", method_line_string_eql, 1);
  rb_define_method(geos_line_string_methods, "hash", method_line_string_hash, 0);
  rb_define_method(geos_line_string_methods, "geometry_type", method_line_string_geometry_type, 0);
  rb_define_method(geos_line_string_methods, "length", method_line_string_length, 0);
  rb_define_method(geos_line_string_methods, "num_points", method_line_string_num_points, 0);
  rb_define_method(geos_line_string_methods, "point_n", method_line_string_point_n, 1);
  rb_define_method(geos_line_string_methods, "points", method_line_string_points, 0);
  rb_define_method(geos_line_string_methods, "start_point", method_line_string_start_point, 0);
  rb_define_method(geos_line_string_methods, "end_point", method_line_string_end_point, 0);
  rb_define_method(geos_line_string_methods, "project_point", method_line_string_project_point, 1);
  rb_define_method(geos_line_string_methods, "interpolate_point", method_line_string_interpolate_point, 1);
  rb_define_method(geos_line_string_methods, "closed?", method_line_string_is_closed, 0);
  rb_define_method(geos_line_string_methods, "ring?", method_line_string_is_ring, 0);
  rb_define_method(geos_line_string_methods, "coordinates", method_line_string_coordinates, 0);

  // CAPILinearRingMethods module
  geos_linear_ring_methods = rb_define_module_under(rgeo_geos_module, "CAPILinearRingMethods");
  rb_define_method(geos_linear_ring_methods, "geometry_type", method_linear_ring_geometry_type, 0);
  rb_define_method(geos_linear_ring_methods, "hash", method_linear_ring_hash, 0);

  // CAPILineMethods module
  geos_line_methods = rb_define_module_under(rgeo_geos_module, "CAPILineMethods");
  rb_define_method(geos_line_methods, "geometry_type", method_line_geometry_type, 0);
  rb_define_method(geos_line_methods, "hash", method_line_hash, 0);
}


VALUE rgeo_is_geos_line_string_closed(GEOSContextHandle_t context, const GEOSGeometry* geom)
{
  VALUE result;
  unsigned int n;
  double x1, x2, y1, y2;
  const GEOSCoordSequence* coord_seq;

  result = Qnil;
  n = GEOSGetNumCoordinates_r(context, geom);
  if (n > 0) {
    coord_seq = GEOSGeom_getCoordSeq_r(context, geom);
    if (GEOSCoordSeq_getX_r(context, coord_seq, 0, &x1)) {
      if (GEOSCoordSeq_getX_r(context, coord_seq, n-1, &x2)) {
        if (x1 == x2) {
          if (GEOSCoordSeq_getY_r(context, coord_seq, 0, &y1)) {
            if (GEOSCoordSeq_getY_r(context, coord_seq, n-1, &y2)) {
              result = y1 == y2 ? Qtrue : Qfalse;
            }
          }
        }
        else {
          result = Qfalse;
        }
      }
    }
  }
  return result;
}


RGEO_END_C

#endif
