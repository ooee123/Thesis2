package ast.declaration;

import ast.expression.Expression;
import ast.type.EnumType;
import ast.type.StructUnionType;
import ast.type.TypedefType;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by ooee on 11/1/16.
 */
@Value
public class TypedefDeclaration implements Declaration {

    private List<TypedefType> typedefTypes;

    public TypedefDeclaration(List<TypedefType> typedefTypes) {
        this.typedefTypes = typedefTypes;
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        return toCode();
    }

    public String toCode() {
        String originalTypeCode;
        TypedefType typedefType = typedefTypes.stream().findAny().get();
        if (typedefType.getOriginalType() instanceof StructUnionType) {
            originalTypeCode = ((StructUnionType) typedefType.getOriginalType()).toExpandedCode();
        } else if (typedefType.getOriginalType() instanceof EnumType) {
            originalTypeCode = ((EnumType) typedefType.getOriginalType()).toExpandedCode();
        } else {
            originalTypeCode = typedefType.getOriginalType().toCode();
        }
        List<String> strings = typedefTypes.stream().map(ty -> ty.getTypedefName()).collect(Collectors.toList());
        return "typedef " + originalTypeCode + " " + String.join(", ", strings) + ";";
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
