# frozen_string_literal: true

# -----------------------------------------------------------------------------
#
# Tests for the GEOS FFI validity implementation
#
# -----------------------------------------------------------------------------

require_relative "../test_helper"

class GeosFFIValidityTest < Minitest::Test # :nodoc:
  include RGeo::Tests::Common::ValidityTests

  def setup
    @factory = RGeo::Geos.factory(native_interface: :ffi)
  end
end if RGeo::Geos.ffi_supported?
