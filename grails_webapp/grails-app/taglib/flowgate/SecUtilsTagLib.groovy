package flowgate

class SecUtilsTagLib {

    def utilsService

    def isOwner = { attrs, body ->
        Object object = attrs?.object
        if(utilsService.isOwnerMember(object, 'owner')) {
            out << body()
        }
    }

    def isOwnerOrRoles = { attrs, body ->
        Object object = attrs?.object
        String roles = attrs?.roles
        if(utilsService.isOwnerMember(object, 'owner') || grails.plugin.springsecurity.SpringSecurityUtils.ifAnyGranted(roles)) {
            out << body()
        }
    }

    def isMember = { attrs, body ->
        String object = attrs?.object
        if(utilsService.isOwnerMember(object, 'member')) {
            out << body()
        }
    }

    def isMemberOrRoles = { attrs, body ->
        Object object = attrs?.object
        String roles = attrs?.roles
        if(utilsService.isOwnerMember(object, 'member') || grails.plugin.springsecurity.SpringSecurityUtils.ifAnyGranted(roles)) {
            out << body()
        }
    }

    def isAffil = { attrs, body ->
        Object object = attrs?.object
        if(utilsService.isAffil(object)) out << body()
    }

    def isAffilOrRoles = { attrs, body ->
        Object object = attrs?.object
        String roles = attrs?.roles
        if(utilsService.isOwnerMember(object, 'owner') || utilsService.isOwnerMember(object, 'member') || grails.plugin.springsecurity.SpringSecurityUtils.ifAnyGranted(roles))
            out << body()
    }

    def isNotAffilOrRoles = { attrs, body ->
        Object object = attrs?.object
        String roles = attrs?.roles
        if(!utilsService.isOwnerMember(object, 'owner') && !utilsService.isOwnerMember(object, 'member') && grails.plugin.springsecurity.SpringSecurityUtils.ifNotGranted(roles))
            out << body()
    }

}
