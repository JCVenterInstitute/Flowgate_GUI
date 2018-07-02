package flowgate

class UrlMappings {

  static mappings = {
    "/register/index"(controller: 'register', action: 'register')
    "/user/newUsers"(controller: 'user', action: 'newUsers')
    "/login/$action?"(controller: "login")
    "/api/login"(controller: "login")
    "/logout/$action?"(controller: "logout")
    "/tokens"(controller: 'authenticationToken')
    "/api/tokens"(resources: "authenticationToken")
    "/mgmnt"(view: "/mgmnt/index")
    "/dashboard"(view: "/dashboard/index")
    "/about"(view: "/about/about")
    "/"(view: "/index")
    //        "/"(controller: 'project', view: 'list' )
    "/project/index"(controller: 'project', action: 'index')
    "/experiment/index"(controller: 'experiment', action: 'index')
//    "/api/projects"(resources: 'project') //carefull! manipulates createLink command controller/index -> api/controller
    //        "/api/experiments/"(resources: 'experiment')
    //        "/api/experiments/$id"(controller: 'experiment', action: 'index')
    "500"(view: '/error')
    "404"(view: '/notFound')
    delete "/$controller/$id(.$format)?"(action: "delete")
    get "/$controller(.$format)?"(action: "index")
    get "/$controller/$id(.$format)?"(action: "show")
    post "/$controller(.$format)?"(action: "save")
    put "/$controller/$id(.$format)?"(action: "update")
    patch "/$controller/$id(.$format)?"(action: "patch")
    "/$controller/$action?/$id?(.$format)?" {
      constraints {
        // apply constraints here
      }
    }
  }
}
