require 'thor/group'
require 'active_support/inflector'
require 'fileutils'
require 'find'

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
      # Construct the correct path to the templates/android directory
      templates_dir = File.join(self.class.source_root, 'templates/android')

      # Iterate over all files in the templates/android/ directory, including subdirectories
      Find.find(templates_dir).each do |file|
        next if File.directory?(file) # Skip directories

        # Calculate the relative path for the destination
        relative_path = file.sub("#{templates_dir}/", '')
        destination_path = "examples/#{name}/android/#{relative_path}"

        template(file, destination_path)
      end
    end

    def print_success_message
      say_status(:success, "Created Android project for example #{friendly_name}", :green)
      say_status(:info, "Please open the examples/#{name}/android folder in Android Studio", :blue)
    end

    private

    def current_year
      @current_year = Time.now.year
    end
  end
end
