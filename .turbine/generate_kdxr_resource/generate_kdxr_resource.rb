require 'thor/group'
require 'active_support/inflector'

module Turbine
  class GenerateKDXrResource < Thor::Group
    include Thor::Actions

    desc "Creates a new KDXr resource including frontend and backend classes"
    argument :name

    def self.source_root
      File.dirname(__FILE__)
    end

    def create_frontend_class
      template('templates/frontend/class.h.erb', "src/KDXr/#{file_name}.h")
      template('templates/frontend/class.cpp.erb', "src/KDXr/#{file_name}.cpp")
    end

    def create_backend_classes
      template('templates/backend/api_class.h.erb', "src/KDXr/api/api_#{file_name}.h")
      template('templates/backend/class.h.erb', "src/KDXr/openxr/openxr_#{file_name}.h")
      template('templates/backend/class.cpp.erb', "src/KDXr/openxr/openxr_#{file_name}.cpp")
    end

    def add_to_cmakelists
      if ::File.exist?("src/KDXr/CMakeLists.txt")
        inject_into_file("src/KDXr/CMakeLists.txt", "    openxr/openxr_#{file_name}.h\n", after: "set(HEADERS\r\n")
        inject_into_file("src/KDXr/CMakeLists.txt", "    api/api_#{file_name}.h\n", after: "set(HEADERS\r\n")
        inject_into_file("src/KDXr/CMakeLists.txt", "    #{file_name}.h\n", after: "set(HEADERS\r\n")

        inject_into_file("src/KDXr/CMakeLists.txt", "    openxr/openxr_#{file_name}.cpp\n", after: "set(SOURCES\r\n")
        inject_into_file("src/KDXr/CMakeLists.txt", "    #{file_name}.cpp\n", after: "set(SOURCES\r\n")
      else
        say_status(:unchanged, "Did not add files to CMakeLists.txt", :blue)
      end
    end

    def print_success_message
      say_status(:success, "Created KDXr resource classes for #{name}", :green)
      say_status(:info, "Please run cmake to configure", :blue)
    end

    private
      def class_name
        name.camelcase
      end

      def variable_name
        name.camelcase(:lower)
      end

      def file_name
        name.underscore.downcase
      end
  end
end
