package ast.type;

import ast.BaseElement;
import ast.expression.ConditionalExpression;
import lombok.Data;
import lombok.NonNull;
import lombok.Setter;
import lombok.Value;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 12/4/16.
 */
@Data
public class EnumType implements ActualType {
    private boolean wasToCode;
    private String tag;
    @NonNull private List<EnumValue> enumValues;
    @Setter private String typedefName;

    @Value
    public class EnumValue implements BaseElement {
        @NonNull private String name;
        private ConditionalExpression value;

        @Override
        public String toCode(boolean showOriginalLine) {
            return toCode();
        }

        public String toCode() {
            if (value != null) {
                return name + "=" + value.toCode();
            } else {
                return name;
            }
        }
    }

    public EnumType(String tag) {
        this.wasToCode = false;
        this.tag = tag;
        this.enumValues = new ArrayList<>();
    }

    public void addEnumValue(String name) {
        addEnumValue(name, null);
    }

    public void addEnumValue(String name, ConditionalExpression value) {
        enumValues.add(new EnumValue(name, value));
    }

    @Override
    public String toCode() {
        if (typedefName != null) {
            return typedefName;
        } else if (tag != null && wasToCode) {
            return "enum " + tag;
        } else {
            return expandedStructUnion();
        }
    }

    public String expandedStructUnion() {
        StringBuilder builder = new StringBuilder();
        builder.append("enum");
        if (tag != null) {
            builder.append(" " + tag);
        }
        if (!wasToCode) {
            wasToCode = true;
            builder.append(" {");
            List<String> enumCodes = enumValues.stream().map(ev -> ev.toCode()).collect(Collectors.toList());
            builder.append(String.join(",", enumCodes));
            builder.append("}");
        }
        return builder.toString();
    }
}
