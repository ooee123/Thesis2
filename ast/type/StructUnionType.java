package ast.type;

import ast.Declaration;
import ast.TypedefDeclaration;
import ast.VariableDeclaration;
import lombok.*;

import java.util.List;

/**
 * Created by ooee on 9/24/16.
 */
@Data
@EqualsAndHashCode(exclude = {"declarations"})
public class StructUnionType extends TypedefType {
    public enum StructUnion {
        STRUCT("struct"),
        UNION("union");

        private String token;

        StructUnion(String token) {
            this.token = token;
        }

        public static StructUnion toStructUnion(String token) {
            for (StructUnion structUnion : StructUnion.values()) {
                if (structUnion.token.equals(token)) {
                    return structUnion;
                }
            }
            throw new IllegalArgumentException("Illegal token of " + token);
        }
    }

    public StructUnionType(String tag, @NonNull StructUnion structUnion, @NonNull List<VariableDeclaration> declarations) {
        this.tag = tag;
        this.structUnion = structUnion;
        this.declarations = declarations;
        this.typedefName = null;
    }

    private final String tag;
    @NonNull private final StructUnion structUnion;
    @NonNull private final List<VariableDeclaration> declarations;
    @Setter private String typedefName;

    public String expandedStructUnion() {
        StringBuilder builder = new StringBuilder();
        builder.append(structUnion.token);
        if (tag != null) {
            builder.append(" " + tag);
        }
        builder.append(" {");
        for (VariableDeclaration declaration : declarations) {
            builder.append(declaration.toCode());
        }
        builder.append("}");
        return builder.toString();
    }

    @Override
    public String toCode() {
        if (typedefName != null) {
            return typedefName;
        } else if (tag != null) {
            return "struct " + tag;
        } else {
            return expandedStructUnion();
        }
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("StructUnionType=(");
        builder.append("tag=" + tag);
        builder.append(", structUnion=" + structUnion);
        builder.append("declarations={");
        for (VariableDeclaration declaration : declarations) {
            for (VariableDeclaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
                String variableType = declaredVariable.getType().toCode();
                String identifier = declaredVariable.getIdentifier().getIdentifier();
                builder.append(variableType + ":" + identifier + ",");
            }
        }
        builder.append("})");
        return builder.toString();
    }
}
