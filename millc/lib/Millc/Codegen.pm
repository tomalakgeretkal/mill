package Millc::Codegen;
use Exporter 'import';
use Modern::Perl;

our @EXPORT_OK = 'codegen';

our $object_builder;
our $bytecode_builder;
our $bytecode_builder_factory;

sub codegen {
    local $object_builder = shift;
    local $bytecode_builder_factory = shift;
    my $module = shift;

    for (@{$module->{decls}}) {
        codegen_decl($_);
    }
}

sub codegen_decl {
    my $decl = shift;
    my %codegens = (
        use_decl => \&codegen_use_decl,
        proc_decl => \&codegen_proc_decl,
        check_decl => \&codegen_check_decl,
        main_decl => \&codegen_main_decl,
    );
    $codegens{$decl->{type}}->($decl);
}

sub codegen_use_decl {
    $object_builder->dependency(join('::', @{shift->{module}}));
}

sub codegen_proc_decl {
    my $proc_decl = shift;
    my $body = '';
    open my $fh, '>:raw', \$body;
    local $bytecode_builder = $bytecode_builder_factory->new($fh);
    codegen_expr($proc_decl->{body});
    $bytecode_builder->pop();
    $bytecode_builder->push_unit();
    $bytecode_builder->return();
    $object_builder->subroutine(
        $proc_decl->{name},
        scalar @{$proc_decl->{params}},
        $body,
    );
}

sub codegen_check_decl {
    my $body = '';
    open my $fh, '>:raw', \$body;
    local $bytecode_builder = $bytecode_builder_factory->new($fh);
    codegen_expr(shift->{body});
    $bytecode_builder->pop();
    $bytecode_builder->push_unit();
    $bytecode_builder->return();
    $object_builder->subroutine('LOAD', 0, $body);
}

sub codegen_main_decl {
    my $body = '';
    open my $fh, '>:raw', \$body;
    local $bytecode_builder = $bytecode_builder_factory->new($fh);
    codegen_expr(shift->{body});
    $bytecode_builder->pop();
    $bytecode_builder->push_unit();
    $bytecode_builder->return();
    $object_builder->subroutine('MAIN', 0, $body);
}

sub codegen_expr {
    my $expr = shift;
    my %codegens = (
        infix_expr => \&codegen_call_expr,
        call_expr => \&codegen_call_expr,
        name_expr => \&codegen_name_expr,
        string_expr => \&codegen_string_expr,
        boolean_expr => \&codegen_boolean_expr,
        block_expr => \&codegen_block_expr,
        if_expr => \&codegen_if_expr,
        check_expr => \&codegen_check_expr,
    );
    $codegens{$expr->{type}}->($expr);
}

sub codegen_infix_expr {
    my $infix_expr = shift;
    codegen_expr($infix_expr->{arguments}->[0]);
    codegen_expr($infix_expr->{callee});
    $bytecode_builder->swap();
    codegen_expr($infix_expr->{arguments}->[1]);
    $bytecode_builder->call(2);
}

sub codegen_call_expr {
    my $call_expr = shift;
    codegen_expr($call_expr->{callee});
    for (@{$call_expr->{arguments}}) {
        codegen_expr($_);
    }
    $bytecode_builder->call(scalar @{$call_expr->{arguments}});
}

sub codegen_name_expr {
    my $name = shift->{name};
    if ($name->{type} eq 'module_member') {
        my $fqname = join('::', @{$name->{module}}, $name->{member});
        my $fqname_id = $object_builder->string($fqname);
        $bytecode_builder->push_global($fqname_id);
    } elsif ($name->{type} eq 'parameter') {
        $bytecode_builder->push_parameter($name->{index});
    } else {
        ...
    }
}

sub codegen_string_expr {
    my $string = shift->{value};
    my $string_id = $object_builder->string($string);
    $bytecode_builder->push_string($string_id);
}

sub codegen_boolean_expr {
    my $value = shift->{value};
    $bytecode_builder->push_boolean($value);
}

sub codegen_block_expr {
    my @stmts = @{shift->{stmts}};
    for (0..$#stmts) {
        codegen_stmt($stmts[$_]);
        if ($_ != $#stmts) {
            $bytecode_builder->pop();
        }
    }
}

sub codegen_if_expr {
    my $if_expr = shift;

    my $then_label = $bytecode_builder->fresh_label();
    my $endif_label = $bytecode_builder->fresh_label();

    codegen_expr($if_expr->{condition});
    $bytecode_builder->conditional_jump($then_label);

    if (defined $if_expr->{else}) {
        codegen_expr($if_expr->{else});
    } else {
        $bytecode_builder->push_unit();
    }
    $bytecode_builder->unconditional_jump($endif_label);

    $bytecode_builder->save_label($then_label);
    codegen_expr($if_expr->{then});

    $bytecode_builder->save_label($endif_label);
}

sub codegen_check_expr {
    my $check_expr = shift;
    codegen_expr($check_expr->{condition});
    $bytecode_builder->check();
}

sub codegen_stmt {
    my $stmt = shift;
    my %codegens = (
        expr_stmt => \&codegen_expr_stmt,
    );
    $codegens{$stmt->{type}}->($stmt);
}

sub codegen_expr_stmt {
    codegen_expr(shift->{expr});
}

1;
