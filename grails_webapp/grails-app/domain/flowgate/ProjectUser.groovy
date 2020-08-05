package flowgate

import grails.gorm.DetachedCriteria
import org.apache.commons.lang.builder.HashCodeBuilder

class ProjectUser implements Serializable {

    Project project
    User user
    String projRole

    @Override
    boolean equals(other) {
        if (other instanceof ProjectUser) {
            other.userId == user?.id && other.projectId == project?.id
        }
    }

    @Override
    int hashCode() {
        def builder = new HashCodeBuilder()
        if (project) builder.append(project.id)
        if (user) builder.append(user.id)
        builder.toHashCode()
    }

    static ProjectUser get(long projectId, long userId, String prole) {
        criteriaFor(projectId, userId, prole).get()
    }

    static boolean exists(long projectId, long userId, String prole) {
        criteriaFor(projectId, userId, prole).count()
    }

    private static DetachedCriteria criteriaFor(long projectId, long userId, String prole) {
        ProjectUser.where {
            project == Project.load(projectId) && user == User.load(userId) && projRole == prole
        }
    }

    static ProjectUser create(Project project, User user, String prole) {
        def instance = new ProjectUser(project: project, user: user, projRole:  prole)
        instance.save()
        instance
    }

    static boolean remove(Project p, User u, String r) {
        if (p != null && u != null && r != '') {
            ProjectUser.where { project == p && user == u && projRole == r }.deleteAll()
        }
    }

    static int removeAll(User u) {
        u == null ? 0 : ProjectUser.where { user == u }.deleteAll()
    }

    static int removeAll(Project p) {
        p == null ? 0 : ProjectUser.where { project == p }.deleteAll()
    }

    static constraints = {
        project validator: {Project p, ProjectUser pu  ->
            if (pu.user?.id) {
                ProjectUser.withNewSession {
                    if (ProjectUser.exists(p.id, pu.user.id, pu.projRole)) {
                        return ['projectUserWithRole.exists']
                    }
                }
            }
        }
    }

    static mapping = {
        id composite: ['project', 'user', 'projRole']
        version false
    }
}
