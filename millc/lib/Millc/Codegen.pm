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
        main_decl => \&codegen_main_decl,
    );
    $codegens{$decl->{type}}->($decl);
}

sub codegen_use_decl {
    $object_builder->dependency(join('::', @{shift->{module}}));
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
        call_expr => \&codegen_call_expr,
        name_expr => \&codegen_name_expr,
        string_expr => \&codegen_string_expr,
        block_expr => \&codegen_block_expr,
    );
    $codegens{$expr->{type}}->($expr);
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
    my $fqname = join('::', @{$name->{module}}, $name->{member});
    my $fqname_id = $object_builder->string($fqname);
    $bytecode_builder->push_global($fqname_id);
}

sub codegen_string_expr {
    my $string = shift->{value};
    my $string_id = $object_builder->string($string);
    $bytecode_builder->push_string($string_id);
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
