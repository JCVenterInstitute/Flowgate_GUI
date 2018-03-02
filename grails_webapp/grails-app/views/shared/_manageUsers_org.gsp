<!-- Modal -->
%{--
<div class="modal fade" id="manageUsersModal-${expFile?.id}" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="false">
    <div class="modal-dialog" role="document">
        <div class="modal-content">
            <div class="modal-header">
                <div class="row">
                    <div class="col-sm-11">
                        <h5 class="modal-title" id="myModalLabel">Manage Users (${expFile?.title})</h5>
                    </div>
                    <div class="col-sm-1 pull-right">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                            <span aria-hidden="true">&times;</span>
                        </button>
                    </div>
                </div>
            </div>
            <div class="modal-body">
                <g:form name="manageUsers" controller="fcsFileContainer" action="manageUsers">
                <div class="row form-horizontal" style="padding-bottom: 10px">
                    <div class="col-sm-2">
                        owner(s)
                    </div>
                    <div class="col-sm-10">
                        <script type="text/javascript">
                            $(document).ready(function () {
                                $(".owners-${expFile?.id}").select2();
                            });
                        </script>
                        <g:select style="width: 100%" class="select2-container owners-${expFile?.id} form-control" name="owners.id" value="${objectUser?.findAllByExpFileAndExpFileRole(expFile, 'owner' )*.user*.id}" from="${User.list()}" optionKey="id" optionValue="username" multiple="" />
                    </div>
                </div>
                <div class="row form-horizontal">
                    <div class="col-sm-2">
                        member(s)
                    </div>
                    <div class="col-sm-10">
                        <script type="text/javascript">
                        $(document).ready(function () {
                            $(".members-${expFile?.id}").select2();
                        });
                        </script>
                        <g:select style="width: 100%" class="select2 multiSelect members-${expFile?.id} form-control" id="members-${expFile?.id}" name="memberSel" value="${objectUser?.findAllByExpFileAndExpFileRole(expFile, 'member' )*.user*.id}" from="${User.list()}" optionKey="id" optionValue="username"  multiple="" />
                    </div>
                </div>
                </g:form>
            </div>
            <div class="modal-footer">
                <button type="button" class="btn btn-secondary" data-dismiss="modal">Close</button>
                <button type="submit" class="btn btn-primary">Save changes</button>
            </div>
        </div>
    </div>
</div>
--}%

<div class="modal fade in" id="manageUsersModal-${expFile?.id}" role="dialog">
    <div class="modal-dialog">

        <div class="modal-content">
            %{--<form class="form-horizontal">--}%
            <g:form class="form-horizontal" name="manageUsers" controller="expFile" action="manageUsers" id="${expFile?.id}">

                <div class="modal-header">
                    <div class="btn-group pull-left">
                        <button class="btn btn-danger" data-dismiss="modal">
                            Cancel
                        </button>
                    </div>
                    <div class="btn-group pull-right">
                        <button type="submit" class="btn btn-success">
                            Save
                        </button>
                    </div>

                    <h3 class="modal-title">Manage Users (${fexpFile?.title})</h3>
                </div>

                <div class="modal-body">
                    %{--
                    <div class="form-group">
                        <label class="col-xs-3 control-label">Name</label>
                        <div class="col-xs-9">
                            <input type="text" name="name" class="form-control" placeholder="Company" value="">
                        </div>
                    </div>
                    <div class="form-group">
                        <label class="col-xs-3 control-label">Type</label>
                        <div class="col-xs-9">
                            <select name="type" class="form-control" value="">
                                <option value="" selected="">Select a Type...</option>
                                <option value="Prospect">Prospect</option>
                                <option value="Customer">Customer</option>
                                <option value="Inactive">Inactive</option>
                            </select>
                        </div>
                    </div>
                    <div class="form-group">
                        <label class="col-xs-3 control-label">Location</label>
                        <div class="col-xs-9">
                            <input type="text" name="location" class="form-control" placeholder="Location" value="">
                        </div>
                    </div>
                </div>
                --}%


                <div class="form-group" style="padding-bottom: 10px">
                    <label class="col-sm-2 control-label">
                        owner(s)
                    </label>
                    <div class="col-sm-10">
                        <script type="text/javascript">
                            $(document).ready(function () {
                                $(".owners-${expFile?.id}").select2();
                            });
                        </script>
                        %{--<g:select style="width: 100%" class="select2-container owners-${expFile?.id} form-control" name="owners" value="${objectUser?.findAllByExpFileAndExpFileRole(expFile, 'owner' )*.user*.id}" from="${User.list()}" optionKey="id" optionValue="username" multiple="" />--}%
                    </div>
                </div>
                <div class="form-group">
                    <label class="col-sm-2 control-label">
                        member(s)
                    </label>
                    <div class="col-sm-10">
                        <script type="text/javascript">
                            $(document).ready(function () {
                                $(".members-${expFile?.id}").select2();
                            });
                        </script>
                        %{--<g:select style="width: 100%" class="select2 multiSelect members-${expFile?.id} form-control" id="members-${expFile?.id}" name="members" value="${objectUser?.findAllByExpFileAndExpFileRole(expFile, 'member' )*.user*.id}" from="${User.list()}" optionKey="id" optionValue="username"  multiple="" />--}%
                    </div>
                </div>


                <div class="modal-footer">
                    %{--
                    <small class="pull-left">Built with Bootcards - Form Card</small>
                    <a href="#" class="btn btn-link btn-xs pull-right">View Source</a>
                    --}%
                </div>

            </g:form>
        </div>

    </div>
</div>


<!-- Button trigger modal -->
%{--<button type="button" class="btn btn-primary" data-toggle="modal" data-target="#exampleModal">--}%
%{--
<button type="button" class="btn btn-primary" data-toggle="modal" data-target="#manageUsersModal-${expFileId}">
    Launch demo modal
</button>
--}%
