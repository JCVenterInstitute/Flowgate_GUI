<div class="row">
  <div class="pull-right btn btn-info" (click)="addBoxContentClick(elbox)">Add</div>
  <div class="btn btn-info" (click)="addBoxContentClick(elbox)"><i class="glyphicon glyphicon-edit"></i>Add</div>
  <div class="btn btn-info" (click)="addBoxContentClick(elbox)"><i class="glyphicon glyphicon-edit"></i>Panel</div>
  <div class="btn btn-info" (click)="addBoxContentClick(elbox)"><i class="glyphicon glyphicon-edit"></i>Reagents</div>
  <div class="btn btn-info" (click)="addBoxContentClick(elbox)"><i class="glyphicon glyphicon-edit"></i>Metadata</div>
  <div class="btn btn-info" (click)="addBoxContentClick(elbox)"><i class="glyphicon glyphicon-edit"></i>Info</div>
  <div class="btn btn-info" (click)="addBoxContentClick(elbox)"><i class="glyphicon glyphicon-edit"></i>Analyses</div>
  <span class="btn btn-info" data-toggle="tooltip" title="manage users" >
    %{--<button type="button" class="" data-toggle="modal" data-target="#manageUsersModal-${expFile?.id}"><i class="fa fa-users"></i></button>--}%
    <i class="fa fa-user" data-toggle="modal" data-target="#manageUsersModal-${expFile?.id}"></i>
  </span>
</div>