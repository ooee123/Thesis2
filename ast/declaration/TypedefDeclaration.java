package ast.declaration;

import ast.Declaration;
import ast.Expression;
import ast.type.StructUnionType;
import ast.type.TypedefType;
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

    private TypedefType typedefType;

    public TypedefDeclaration(TypedefType typedefType) {
        this.typedefType = typedefType;
    }

    @Override
    public String toCode() {
        return "typedef " + typedefType.getOriginalType().toCode() + " " + typedefType.getTypedefName() + ";";
    }

    @Override
    public boolean isCritical() {
        return false;
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
