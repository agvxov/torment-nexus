#include <unordered_map>

template<typename T>
class resource_manager {
    T (*load_resource)(const char * s);
    bool (*is_resource_valid)(T r);
    void (*unload_resource)(T r);

    std::unordered_map<const char *, T> resources;
    T default_resource;

  public:
    resource_manager(
      T (*load_resource_)(const char * s),
      const char * default_path,
      bool (*is_resource_valid_)(T r),
      void (*unload_resource_)(T r)
    )
      : load_resource(load_resource_)
      , is_resource_valid(is_resource_valid_)
      , unload_resource(unload_resource_)
    {
        default_resource = load_resource(default_path);
    }

    ~resource_manager() {
        for (auto [k, v] : resources) {
            unload_resource(v);
        }
    }

    T load(const char * id) {
        T r = load_resource(id);
        if (!is_resource_valid(r)) {
            r = default_resource;
        }
        resources[id] = r;

        return r;
    }

    T get(const char * id) {
        if (resources.find(id) != resources.end()) {
            return resources[id];
        }

        return load(id);
    }
};
