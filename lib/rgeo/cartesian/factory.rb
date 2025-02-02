# frozen_string_literal: true

# -----------------------------------------------------------------------------
#
# Geographic data factory implementation
#
# -----------------------------------------------------------------------------

module RGeo
  module Cartesian
    # This class implements the factory for the simple cartesian
    # implementation.

    class Factory
      include Feature::Factory::Instance
      include ImplHelper::Utils

      # Create a new simple cartesian factory.
      #
      # See RGeo::Cartesian.simple_factory for a list of supported options.

      def initialize(opts = {})
        @has_z = opts[:has_z_coordinate] ? true : false
        @has_m = opts[:has_m_coordinate] ? true : false
        @proj4 = opts[:proj4]
        if @proj4 && CoordSys.check!(:proj4)
          if @proj4.is_a?(String) || @proj4.is_a?(Hash)
            @proj4 = CoordSys::Proj4.create(@proj4)
          end
        end
        srid = opts[:srid]
        @coord_sys = opts[:coord_sys]
        if @coord_sys.is_a?(String)
          @coord_sys = CoordSys::CS.create_from_wkt(@coord_sys)
        end
        if (!@proj4 || !@coord_sys) && srid && (db = opts[:srs_database])
          entry = db.get(srid.to_i)
          if entry
            @proj4 ||= entry.proj4
            @coord_sys ||= entry.coord_sys
          end
        end
        srid ||= @coord_sys.authority_code if @coord_sys
        @srid = srid.to_i
        @buffer_resolution = opts[:buffer_resolution].to_i
        @buffer_resolution = 1 if @buffer_resolution < 1

        wkt_generator = opts[:wkt_generator]
        case wkt_generator
        when Hash
          @wkt_generator = WKRep::WKTGenerator.new(wkt_generator)
        else
          @wkt_generator = WKRep::WKTGenerator.new(convert_case: :upper)
        end
        wkb_generator = opts[:wkb_generator]
        case wkb_generator
        when Hash
          @wkb_generator = WKRep::WKBGenerator.new(wkb_generator)
        else
          @wkb_generator = WKRep::WKBGenerator.new
        end
        wkt_parser = opts[:wkt_parser]
        case wkt_parser
        when Hash
          @wkt_parser = WKRep::WKTParser.new(self, wkt_parser)
        else
          @wkt_parser = WKRep::WKTParser.new(self)
        end
        wkb_parser = opts[:wkb_parser]
        case wkb_parser
        when Hash
          @wkb_parser = WKRep::WKBParser.new(self, wkb_parser)
        else
          @wkb_parser = WKRep::WKBParser.new(self)
        end
      end

      # Equivalence test.

      def eql?(rhs)
        rhs.is_a?(self.class) && @srid == rhs.srid &&
          @has_z == rhs.property(:has_z_coordinate) &&
          @has_m == rhs.property(:has_m_coordinate) &&
          @proj4.eql?(rhs.proj4)
      end
      alias == eql?

      # Standard hash code

      def hash
        @hash ||= [@srid, @has_z, @has_m, @proj4].hash
      end

      # Marshal support

      def marshal_dump # :nodoc:
        hash_ = {
          "hasz" => @has_z,
          "hasm" => @has_m,
          "srid" => @srid,
          "wktg" => @wkt_generator.properties,
          "wkbg" => @wkb_generator.properties,
          "wktp" => @wkt_parser.properties,
          "wkbp" => @wkb_parser.properties,
          "bufr" => @buffer_resolution
        }
        hash_["proj4"] = @proj4.marshal_dump if @proj4
        hash_["cs"] = @coord_sys.to_wkt if @coord_sys
        hash_
      end

      def marshal_load(data) # :nodoc:
        if (proj4_data = data["proj4"]) && CoordSys.check!(:proj4)
          proj4 = CoordSys::Proj4.allocate
          proj4.marshal_load(proj4_data)
        else
          proj4 = nil
        end
        if (coord_sys_data = data["cs"])
          coord_sys = CoordSys::CS.create_from_wkt(coord_sys_data)
        else
          coord_sys = nil
        end
        initialize(
          has_z_coordinate: data["hasz"],
          has_m_coordinate: data["hasm"],
          srid: data["srid"],
          wkt_generator: symbolize_hash(data["wktg"]),
          wkb_generator: symbolize_hash(data["wkbg"]),
          wkt_parser: symbolize_hash(data["wktp"]),
          wkb_parser: symbolize_hash(data["wkbp"]),
          buffer_resolution: data["bufr"],
          proj4: proj4,
          coord_sys: coord_sys
        )
      end

      # Psych support

      def encode_with(coder) # :nodoc:
        coder["has_z_coordinate"] = @has_z
        coder["has_m_coordinate"] = @has_m
        coder["srid"] = @srid
        coder["buffer_resolution"] = @buffer_resolution
        coder["wkt_generator"] = @wkt_generator.properties
        coder["wkb_generator"] = @wkb_generator.properties
        coder["wkt_parser"] = @wkt_parser.properties
        coder["wkb_parser"] = @wkb_parser.properties
        if @proj4
          str = @proj4.original_str || @proj4.canonical_str
          coder["proj4"] = @proj4.radians? ? { "proj4" => str, "radians" => true } : str
        end
        coder["coord_sys"] = @coord_sys.to_wkt if @coord_sys
      end

      def init_with(coder) # :nodoc:
        if (proj4_data = coder["proj4"]) && CoordSys.check!(:proj4)
          if proj4_data.is_a?(Hash)
            proj4 = CoordSys::Proj4.create(proj4_data["proj4"], radians: proj4_data["radians"])
          else
            proj4 = CoordSys::Proj4.create(proj4_data.to_s)
          end
        else
          proj4 = nil
        end
        if (coord_sys_data = coder["cs"])
          coord_sys = CoordSys::CS.create_from_wkt(coord_sys_data.to_s)
        else
          coord_sys = nil
        end
        initialize(
          has_z_coordinate: coder["has_z_coordinate"],
          has_m_coordinate: coder["has_m_coordinate"],
          srid: coder["srid"],
          wkt_generator: symbolize_hash(coder["wkt_generator"]),
          wkb_generator: symbolize_hash(coder["wkb_generator"]),
          wkt_parser: symbolize_hash(coder["wkt_parser"]),
          wkb_parser: symbolize_hash(coder["wkb_parser"]),
          buffer_resolution: coder["buffer_resolution"],
          proj4: proj4,
          coord_sys: coord_sys
        )
      end

      # Returns the SRID.

      attr_reader :srid

      # See RGeo::Feature::Factory#property

      def property(name)
        case name
        when :has_z_coordinate
          @has_z
        when :has_m_coordinate
          @has_m
        when :buffer_resolution
          @buffer_resolution
        when :is_cartesian
          true
        end
      end

      # See RGeo::Feature::Factory#parse_wkt

      def parse_wkt(str)
        @wkt_parser.parse(str)
      end

      # See RGeo::Feature::Factory#parse_wkb

      def parse_wkb(str)
        @wkb_parser.parse(str)
      end

      # See RGeo::Feature::Factory#point

      def point(x, y, *extra)
        PointImpl.new(self, x, y, *extra)
      end

      # See RGeo::Feature::Factory#line_string

      def line_string(points)
        LineStringImpl.new(self, points)
      end

      # See RGeo::Feature::Factory#line

      def line(start, stop)
        LineImpl.new(self, start, stop)
      end

      # See RGeo::Feature::Factory#linear_ring

      def linear_ring(points)
        LinearRingImpl.new(self, points)
      end

      # See RGeo::Feature::Factory#polygon

      def polygon(outer_ring, inner_rings = nil)
        PolygonImpl.new(self, outer_ring, inner_rings)
      end

      # See RGeo::Feature::Factory#collection

      def collection(elems)
        GeometryCollectionImpl.new(self, elems)
      end

      # See RGeo::Feature::Factory#multi_point

      def multi_point(elems)
        MultiPointImpl.new(self, elems)
      end

      # See RGeo::Feature::Factory#multi_line_string

      def multi_line_string(elems)
        MultiLineStringImpl.new(self, elems)
      end

      # See RGeo::Feature::Factory#multi_polygon

      def multi_polygon(elems)
        MultiPolygonImpl.new(self, elems)
      end

      # See RGeo::Feature::Factory#proj4

      attr_reader :proj4

      # See RGeo::Feature::Factory#coord_sys

      attr_reader :coord_sys

      def generate_wkt(obj)
        @wkt_generator.generate(obj)
      end

      def generate_wkb(obj)
        @wkb_generator.generate(obj)
      end

      def marshal_wkb_generator
        @marshal_wkb_generator ||= RGeo::WKRep::WKBGenerator.new(type_format: :wkb12)
      end

      def marshal_wkb_parser
        @marshal_wkb_parser ||= RGeo::WKRep::WKBParser.new(self, support_wkb12: true)
      end

      def psych_wkt_generator
        @psych_wkt_generator ||= RGeo::WKRep::WKTGenerator.new(tag_format: :wkt12)
      end

      def psych_wkt_parser
        @psych_wkt_parser ||= RGeo::WKRep::WKTParser.new(self, support_wkt12: true, support_ewkt: true)
      end
    end
  end
end
