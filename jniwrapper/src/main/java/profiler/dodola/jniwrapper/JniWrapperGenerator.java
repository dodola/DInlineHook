package profiler.dodola.jniwrapper;

import android.content.Context;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class JniWrapperGenerator {
    private static class Item {
        public Class cls;
        public boolean includePrivate;
        public boolean recursive;
        public boolean stubOnly;

        public Item() {
        }

        public Item(Class cls, boolean includePrivate, boolean recursive) {
            this.cls = cls;
            this.includePrivate = includePrivate;
            this.recursive = recursive;
        }
    }

    private Item item;
    private Class cls;
    private String defClsName;
    private Map<Class, String> usedClasses = new HashMap<Class, String>();
    private Set<Class> done = new HashSet<Class>();
    private Map<String, AccessibleObject> usedNames = new HashMap<String, AccessibleObject>();
    private List<Item> todo = new ArrayList<Item>();

    private StringBuffer header = new StringBuffer();
    private StringBuffer declaration = new StringBuffer();
    private StringBuffer implementation = new StringBuffer();

    public JniWrapperGenerator() throws Exception {
    }

    private String convertClassName(Class cls) {
//        StringBuffer res = new StringBuffer();
//        for (String i : cls.getName().replace('$', '_').split("\\.")) {
//            res.append(i.substring(0, 1).toUpperCase() + i.substring(1));
//        }
//        if (item.recursive) {
//            if (!done.contains(cls)) {
//                todo.add(new Item(cls, item.includePrivate, item.recursive));
//            }
//        }
//        usedClasses.put(cls, res.toString());
//        return res.toString();
        return cls.getSimpleName();
    }

    private static String getSignature(Class cls) throws Exception {
        if (cls.isPrimitive()) {
            if (cls == void.class) return "V";
            if (cls == byte.class) return "B";
            if (cls == short.class) return "S";
            if (cls == int.class) return "I";
            if (cls == long.class) return "J";
            if (cls == float.class) return "F";
            if (cls == double.class) return "D";
            if (cls == char.class) return "C";
            if (cls == boolean.class) return "Z";
            throw new Exception("cannot convert: " + cls);
        } else if (cls.isArray()) {
            return "[" + getSignature(cls.getComponentType());
        } else {
            return "L" + cls.getName().replace(".", "/") + ";";
        }
    }

    private static String getSignature(Method member) throws Exception {
        StringBuffer res = new StringBuffer();
        res.append("(");
        for (Class i : member.getParameterTypes()) {
            res.append(getSignature(i));
        }
        res.append(")");
        res.append(getSignature(member.getReturnType()));
        return res.toString();
    }

    private static String getSignature(Constructor member) throws Exception {
        StringBuffer res = new StringBuffer();
        res.append("(");
        for (Class i : member.getParameterTypes()) {
            res.append(getSignature(i));
        }
        res.append(")V");
        return res.toString();
    }

    private String getNativeClassName(Class cls) throws Exception {
        if (cls.isPrimitive()) {
            if (cls == void.class) return "void";
            return "j" + cls.getName();
        } else if (cls.isArray()) {
            return "jniext::Array<" + getNativeClassName(cls.getComponentType()) + ">";
        } else {
            return convertClassName(cls);
        }
    }

    private String getNativeClassNameRef(Class cls) throws Exception {
        if (cls.isPrimitive()) {
            return getNativeClassName(cls);
        } else {
            return "const jniext::Ref<" + getNativeClassName(cls) + ">&";
        }
    }

    private String getNativeClassNameLocalRef(Class cls) throws Exception {
        if (cls.isPrimitive()) {
            return getNativeClassName(cls);
        } else {
            return "jniext::LocalRef<" + getNativeClassName(cls) + ">";
        }
    }

    private String castParameter(Class cls, String name) throws Exception {
        if (cls.isPrimitive()) return name;
        if (cls == String.class) return "(jstring)" + name;
        return "(jobject)" + name;
    }

    private String mangleName(String name, AccessibleObject obj) {
        if (name.equals("and") || name.equals("or") || name.equals("not") || name.equals("xor"))
            name += "_";
        if (name.equals("register") || name.equals("virtual") || name.equals("union")) name += "_";
        if (name.equals("delete")) name += "_";
        if (name.equals("parent")) name += "_";
        if (name.equals("clazz")) name += "_";
        while (usedNames.containsKey(name) && !usedNames.get(name).getClass().equals(obj.getClass()))
            name += "_";
        usedNames.put(name, obj);
        return name;
    }

    private void handleGetClass() throws Exception {
        declaration.append("\n");
        declaration.append("    static const char *clazz();\n");

//        implementation.append("jniext::Class>& " + defClsName + "::clazz() {\n");
//        implementation.append("    static jniext::GlobalRef<jniext::Class> cls;\n");
//        implementation.append("    if (!cls) cls.set(jniext::Class::forName(\"" + cls.getName().replace(".", "/") + "\"));\n");
//        implementation.append("    return cls;\n");
//        implementation.append("}\n");
    }

    private void handleMethod(Method member) throws Exception {
        Class retType = member.getReturnType();
        Class[] paramTypes = member.getParameterTypes();
        String name = member.getName();
        String defName = mangleName(name, member);

        declaration.append("\n");
        declaration.append("    // " + member + "\n");
        declaration.append("    " + getNativeClassNameLocalRef(retType) + " " + defName + "(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) declaration.append(", ");
            declaration.append(getNativeClassNameRef(paramTypes[i]) + " a" + i);
        }
        declaration.append(") const;\n");

        implementation.append("\n");
        implementation.append("// " + member + "\n");
        implementation.append(getNativeClassNameLocalRef(retType) + " " + defClsName + "::" + defName + "(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) implementation.append(", ");
            implementation.append(getNativeClassNameRef(paramTypes[i]) + " a" + i);
        }
        implementation.append(") const {\n");

        if (cls == Class.class && member.getName() == "isAssignableFrom") {
            implementation.append("    return jniext::Env::get()->IsAssignableFrom(a0, *this);\n");
        } else if (cls == Class.class && member.getName() == "isInstance") {
            implementation.append("    return jniext::Env::get()->IsInstanceOf(a0, *this);\n");
        } else {
            implementation.append("    static jniext::Method<" + getNativeClassName(retType));
            for (int i = 0; i < paramTypes.length; i++) {
                implementation.append(",");
                implementation.append(getNativeClassName(paramTypes[i]));
            }
            implementation.append("> method(clazz(), \"" + member.getName() + "\", \"" + getSignature(member) + "\");\n");
            implementation.append("    ");
            if (retType != void.class) implementation.append("return ");
            implementation.append("method.call(*this");
            for (int i = 0; i < paramTypes.length; i++) {
                implementation.append(", a" + i);
            }
            implementation.append(");\n");
        }
        implementation.append("}\n");
    }

    private void handleStaticMethod(Method member) throws Exception {
        Class retType = member.getReturnType();
        Class[] paramTypes = member.getParameterTypes();
        String defName = mangleName(member.getName(), member);

        declaration.append("\n");
        declaration.append("    // " + member + "\n");
        declaration.append("    static " + getNativeClassNameLocalRef(retType) + " " + defName + "(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) declaration.append(", ");
            declaration.append(getNativeClassNameRef(paramTypes[i]) + " a" + i);
        }
        declaration.append(");\n");

        implementation.append("\n");
        implementation.append("// " + member + "\n");
        implementation.append(getNativeClassNameLocalRef(retType) + " " + defClsName + "::" + defName + "(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) implementation.append(", ");
            implementation.append(getNativeClassNameRef(paramTypes[i]) + " a" + i);
        }
        implementation.append(") {\n");
        implementation.append("    static jniext::StaticMethod<" + getNativeClassName(retType));
        for (int i = 0; i < paramTypes.length; i++) {
            implementation.append(",");
            implementation.append(getNativeClassName(paramTypes[i]));
        }
        implementation.append("> method(clazz(), \"" + member.getName() + "\", \"" + getSignature(member) + "\");\n");
        implementation.append("    ");
        if (retType != void.class) implementation.append("return ");
        implementation.append("method.call(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) implementation.append(", ");
            implementation.append("a" + i);
        }
        implementation.append(");\n");
        implementation.append("}\n");
    }

    private void handleConstructor(Constructor member) throws Exception {
        Class[] paramTypes = member.getParameterTypes();
        declaration.append("\n");
        declaration.append("    // " + member + "\n");
        declaration.append("    static " + getNativeClassNameLocalRef(cls) + " construct(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) declaration.append(", ");
            declaration.append(getNativeClassNameRef(paramTypes[i]) + " a" + i);
        }
        declaration.append(");\n");

        implementation.append("\n");
        implementation.append("// " + member + "\n");
        implementation.append(getNativeClassNameLocalRef(cls) + " " + defClsName + "::construct(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) implementation.append(", ");
            implementation.append(getNativeClassNameRef(paramTypes[i]) + " a" + i);
        }
        implementation.append(") {\n");
        implementation.append("    static jniext::Constructor<" + getNativeClassName(cls));
        for (int i = 0; i < paramTypes.length; i++) {
            implementation.append(",");
            implementation.append(getNativeClassName(paramTypes[i]));
        }
        implementation.append("> constructor(clazz(), \"" + getSignature(member) + "\");\n");
        implementation.append("    return constructor.construct(");
        for (int i = 0; i < paramTypes.length; i++) {
            if (i > 0) implementation.append(", ");
            implementation.append("a" + i);
        }
        implementation.append(");\n");
        implementation.append("}\n");
    }

    private void handleStaticField(Field member) throws Exception {
        Class type = member.getType();
        String defName = mangleName(member.getName(), member);
        declaration.append("\n");
        declaration.append("#pragma push_macro(\"" + member.getName() + "\")\n");
        declaration.append("#undef " + member.getName() + "\n");
        declaration.append("    // " + member + "\n");
        declaration.append("    static ");
        if ((member.getModifiers() & Modifier.FINAL) != 0) declaration.append("const ");
        declaration.append("jniext::StaticField<" + getNativeClassName(type) + "> " + defName + ";\n");
        declaration.append("#pragma pop_macro(\"" + member.getName() + "\")\n");

        implementation.append("#undef " + member.getName() + "\n");
        if ((member.getModifiers() & Modifier.FINAL) != 0) implementation.append("const ");
        implementation.append("jniext::StaticField<" + getNativeClassName(type) + "> ");
        implementation.append(convertClassName(cls) + "::" + defName);
        implementation.append(" (\"" + cls.getName().replace(".", "/") + "\", \"" + member.getName() + "\", \"" + getSignature(type) + "\");\n");
    }

    private void handleField(Field member) throws Exception {
        Class type = member.getType();
        String defName = mangleName(member.getName(), member);
        declaration.append("\n");
        declaration.append("    // " + member + "\n");
        declaration.append("    ");
        if ((member.getModifiers() & Modifier.FINAL) != 0) declaration.append("const ");
        declaration.append("jniext::BoundField<" + getNativeClassName(type) + "> " + defName + " = ");
        declaration.append("jniext::BoundField<" + getNativeClassName(type) + ">(\"" + cls.getName().replace(".", "/") + "\", \"" + member.getName() + "\", \"" + getSignature(type) + "\", this);\n");
        // @TODO...
    }

    private void handleClass(Item item) throws Exception {
        //if (done.contains(cls)) return;
        this.item = item;
        this.cls = item.cls;
        done.add(cls);
        defClsName = convertClassName(cls);

        usedNames.clear();

        declaration.append("\n");
        declaration.append("// " + cls + (item.stubOnly ? " (stub)" : "") + "\n");

        String parent = (cls.getSuperclass() == null) ? "jniext::Object" : getNativeClassName(cls.getSuperclass());
        if (cls.equals(Class.class)) parent = "jniext::Class";
        if (cls.equals(String.class)) parent = "jniext::String";

        declaration.append("class " + defClsName + " : public " + parent + "\n");
        declaration.append("{\n");
        declaration.append("public:\n");
        declaration.append("    using parent = " + parent + ";\n");
        declaration.append("    using parent::parent;\n");

        handleGetClass();

        if (!item.stubOnly) {

            for (Constructor member : cls.getDeclaredConstructors()) {
                handleConstructor(member);
            }

            for (Method member : cls.getDeclaredMethods()) {

                if (member.getName().equals("toString")) continue;
                if (member.getName().equals("getClass")) continue;

                if (!item.includePrivate && (member.getModifiers() & Modifier.PUBLIC) == 0)
                    continue;

                if (member.isSynthetic()) continue;

                if ((member.getModifiers() & Modifier.STATIC) != 0) {
                    handleStaticMethod(member);
                } else {
                    handleMethod(member);
                }
            }

            for (Field member : cls.getDeclaredFields()) {
                if (!item.includePrivate && (member.getModifiers() & Modifier.PUBLIC) == 0)
                    continue;
                if ((member.getModifiers() & Modifier.STATIC) != 0) {
                    handleStaticField(member);
                } else {
                    handleField(member);
                }
            }

            // @TODO: nested classes

            // @TODO: gather all dependencies

        }
        declaration.append("};\n");
    }

    void run(String[] args, Context context) throws Exception {
        for (String arg : args) {
            Item item = new Item();
            if (arg.endsWith(":*")) {
                arg = arg.substring(0, arg.length() - 2);
                item.recursive = true;
            }
            if (arg.endsWith(":p")) {
                arg = arg.substring(0, arg.length() - 2);
                item.includePrivate = true;
            }
            item.cls = Class.forName(arg);
            if (item.recursive) {
                todo.add(0, item);
            } else {
                todo.add(item);
            }
        }
        while (true) {
            while (todo.size() > 0) {
                Item item = todo.remove(0);
                if (done.contains(item.cls)) continue;
                Class scls = item.cls.getSuperclass();
                if (scls != null && !done.contains(scls)) {
                    todo.add(0, item);
                    Item sitem = new Item();
                    sitem.cls = scls;
                    sitem.stubOnly = item.stubOnly;
                    todo.add(0, sitem);
                    continue;
                }
                System.err.println(item.cls);
                handleClass(item);
            }
            for (Class cls : new ArrayList<Class>(usedClasses.keySet())) {
                if (done.contains(cls)) continue;
                Item item = new Item();
                item.cls = cls;
                item.stubOnly = true;
                todo.add(item);
            }
            if (todo.size() == 0) break;
        }
        String absolutePath = context.getFilesDir().getAbsolutePath();
        {
            StringBuffer res = new StringBuffer();
            res.append("#pragma once\n");
            res.append("#include \"jniext/jniext.h\"\n");
            res.append("\n");
            for (String item : usedClasses.values()) {
                res.append("class " + item + ";\n");
            }
            res.append("\n");
            res.append(declaration);
            res.append("\n");

            FileWriter writer = new FileWriter(absolutePath + "/" + "_java.h");
            writer.write(res.toString());
            writer.flush();
        }
        {
            StringBuffer res = new StringBuffer();
            res.append("#include \"_java.h\"\n");
            res.append("\n");
            res.append(implementation);
            res.append("\n");
            FileWriter writer = new FileWriter(absolutePath + "/" + "_java.cpp");
            writer.write(res.toString());
            writer.flush();
        }
    }

    public static void main(Context context, String[] args) throws Exception {
        new JniWrapperGenerator().run(args, context);
    }
}
