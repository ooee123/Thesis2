package ast.declaration;

import ast.Declaration;
import ast.expression.Expression;
import ast.type.EnumType;
import ast.type.StructUnionType;
import ast.type.TypedefType;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

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
        String originalTypeCode;
        if (typedefType.getOriginalType() instanceof StructUnionType) {
            originalTypeCode = ((StructUnionType) typedefType.getOriginalType()).expandedStructUnion();
        } else if (typedefType.getOriginalType() instanceof EnumType) {
            originalTypeCode = ((EnumType) typedefType.getOriginalType()).expandedStructUnion();
        } else {
            originalTypeCode = typedefType.getOriginalType().toCode();
        }
        return "typedef " + originalTypeCode + " " + typedefType.getTypedefName() + ";";
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
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        return;
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitOwnedExpressions(visitor);
    }
}
