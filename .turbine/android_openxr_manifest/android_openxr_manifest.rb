require 'thor/group'
require 'active_support/inflector'
require 'fileutils'

module Turbine
  class AndroidOpenXRManifest < Thor::Group
    include Thor::Actions

    desc "Generates the manifest for an Android OpenXR project"
    argument :name

    def self.source_root
      File.dirname(__FILE__)
    end

    def create_android_dir
      template('templates/android/app/src/main/AndroidManifest.xml.erb', "examples/#{name}/android/app/src/main/AndroidManifest.xml")
    end

    def print_success_message
      say_status(:success, "Created Android manifest for example #{name}", :green)
      say_status(:info, "Please open the examples/#{name}/android folder in Android Studio", :blue)
    end

    private

  end
end
