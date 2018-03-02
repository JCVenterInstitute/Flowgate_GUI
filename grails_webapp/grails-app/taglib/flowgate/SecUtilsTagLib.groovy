package flowgate

//import grails.plugin.springsecurity.SpringSecurityUtils

class SecUtilsTagLib {
//    static defaultEncodeAs = [taglib:'html']
    //static encodeAsForTags = [tagName: [taglib:'html'], otherTagName: [taglib:'none']]

    def springSecurityService

    def isOwner = { attrs, body ->
        String object = attrs?.object
        Long objectId = attrs?.objectId?.toLong()
        if(isOwnerMember(object, objectId, 'owner')) {
            out << body()
        }
    }

    def isOwnerOrRoles = { attrs, body ->
        String object = attrs?.object
        Long objectId = attrs?.objectId?.toLong()
        String roles = attrs?.roles
        if(isOwnerMember(object, objectId, 'owner') || grails.plugin.springsecurity.SpringSecurityUtils.ifAnyGranted(roles)) {
            out << body()
        }
    }

    def isMember = { attrs, body ->
        String object = attrs?.object
        Long objectId = attrs?.objectId?.toLong()
        if(isOwnerMember(object, objectId, 'member')) {
            out << body()
        }
    }

    def isMemberOrRoles = { attrs, body ->
        String object = attrs?.object
        Long objectId = attrs?.objectId?.toLong()
        String roles = attrs?.roles
        if(isOwnerMember(object, objectId, 'member') || grails.plugin.springsecurity.SpringSecurityUtils.ifAnyGranted(roles)) {
            out << body()
        }
    }

    def isAffil = { attrs, body ->
        String object = attrs?.object
        Long objectId = attrs?.objectId?.toLong()
        if(isOwnerMember(object, objectId, 'owner') || isOwnerMember(object, objectId, 'member')) out << body()
    }

    def isAffilOrRoles = { attrs, body ->
        String object = attrs?.object
        Long objectId = attrs?.objectId?.toLong()
        String roles = attrs?.roles
        Boolean dum1 = isOwnerMember(object, objectId, 'owner')
        Boolean dum2 = isOwnerMember(object, objectId, 'member')
        Boolean dum3 = grails.plugin.springsecurity.SpringSecurityUtils.ifAnyGranted(roles)
        if(isOwnerMember(object, objectId, 'owner') || isOwnerMember(object, objectId, 'member') || grails.plugin.springsecurity.SpringSecurityUtils.ifAnyGranted(roles))
            out << body()
    }

    def isNotAffilOrRoles = { attrs, body ->
        String object = attrs?.object
        Long objectId = attrs?.objectId?.toLong()
        String roles = attrs?.roles
        if(!isOwnerMember(object, objectId, 'owner') && !isOwnerMember(object, objectId, 'member') && grails.plugin.springsecurity.SpringSecurityUtils.ifNotGranted(roles))
            out << body()
    }

    def isOwnerMember (String object, Long objectId, String role){
        if (object){
            switch (object) {
                case 'project': return (ProjectUser.findAllByProjectAndUser(Project.get(objectId), springSecurityService.currentUser)*.projRole).contains(role)
                    break
                case 'experiment': return (ExperimentUser.findAllByExperimentAndUser(Experiment.get(objectId), springSecurityService.currentUser)*.expRole).contains(role)
                    break
            }
        }
        return false
    }

    /*
  def isOwner = { attrs, body ->
      def loggedInUser = springSecurityService.currentUser
      def owner = attrs?.owner

      if(loggedInUser?.id == owner?.id) {
          out << body()
      }
  }
  */

    //  use it like:
    /*
    <g:isOwner owner="${post?.author}">
        <g:link controller="post" action="edit" id="${post.id}">
            Edit this post
        </g:link>
    </g:isOwner>
    */


}
