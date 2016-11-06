package ast;

import ast.type.StructUnionType;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 11/5/16.
 */
@Value
public class StructDeclaration implements Declaration {
    private StructUnionType structUnionType;
    @Override
    public String toCode() {
        return structUnionType.expandedStructUnion() + ";";
    }

    @Override
    public boolean isCritical() {
        return true;
    }

    @Override
    public Set<String> getDependantVariables() {
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        return;
    }
}
