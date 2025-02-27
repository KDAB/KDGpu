require 'thor/group'
require 'active_support/inflector'
require 'fileutils'

module Turbine
  class GenerateAndroidExample < Thor::Group
    include Thor::Actions

    desc "Generates the skeleton of an Android project for an existing example"
    argument :name
    argument :friendly_name

    def self.source_root
      File.dirname(__FILE__)
    end

    def create_android_dir
      directory("files/android/.", "examples/#{name}/android")

      template('templates/android/CMakeLists.txt.erb', "examples/#{name}/android/CMakeLists.txt")
      template('templates/android/settings.gradle.kts.erb', "examples/#{name}/android/settings.gradle.kts")
      template('templates/android/app/build.gradle.kts.erb', "examples/#{name}/android/app/build.gradle.kts")
      template('templates/android/app/src/main/AndroidManifest.xml.erb', "examples/#{name}/android/app/src/main/AndroidManifest.xml")
      template('templates/android/app/src/main/res/values/strings.xml.erb', "examples/#{name}/android/app/src/main/res/values/strings.xml")
    end

    def print_success_message
      say_status(:success, "Created Android project for example #{friendly_name}", :green)
      say_status(:info, "Please open the examples/#{name}/android folder in Android Studio", :blue)
    end

    private

  end
end
