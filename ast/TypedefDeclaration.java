package ast;

import ast.type.StructUnionType;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.List;
import java.util.Set;

/**
 * Created by ooee on 11/1/16.
 */
@Value
public class TypedefDeclaration implements Declaration {

    private StructUnionType structUnionType;
    private String typedefName;

    public TypedefDeclaration(StructUnionType structUnionType, String typedefName) {
        this.structUnionType = structUnionType;
        this.typedefName = typedefName;
        structUnionType.setTypedefName(typedefName);
    }

    @Override
    public String toCode() {
        return "typedef " + structUnionType.expandedStructUnion() + " " + typedefName + ";";
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
